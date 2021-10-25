#include "window.h"
#include <QGuiApplication>
#include <QKeyEvent>
#include <QImageReader>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QScreen>
#include <QFontDatabase>
#include <QRandomGenerator>

#include <unistd.h>

#define MAX_SIZE 5000

Window::Window()
{
    setFlag(Qt::Dialog);
    setFlag(Qt::FramelessWindowHint);
    setOpacity(0.5);

    setMinimumSize(QSize(200, 120));
    setMaximumSize(screen()->availableSize().shrunkBy(QMargins(200, 200, 200, 200)));

    m_background = QPixmap(20, 20);
    QPainter pmp(&m_background);
    pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
    pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
    pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
    pmp.fillRect(10, 0, 10, 10, Qt::darkGray);

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(15);
    m_textHeight = QFontMetrics(font).height();
    qApp->setFont(font);

    QMetaObject::invokeMethod(this, &Window::ensureVisible, Qt::QueuedConnection);
}
Window::~Window()
{
    if (sock != -1) {
        char tmp = '\0';
        ::write(sock, &tmp, 1);
        ::close(sock);
        sock = -1;
    }
}

bool Window::setLocalImage(const QString &filename)
{
    if (filename == "/dev/null") {
        return true;
    }
    m_localFile = filename;
    return loadImage(filename, &m_localImage, &m_localSize);
}

bool Window::setRemoteImage(const QString &filename)
{
    if (filename == "/dev/null") {
        return true;
    }
    m_remoteFile = filename;
    return loadImage(filename, &m_remoteImage, &m_remoteSize);
}

void Window::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Q:
    case Qt::Key_Escape:
        if (m_outputFilename.isEmpty()) {
            qApp->exit(0);
        } else {
            qApp->exit(6);
        }
        break;
    case Qt::Key_S: {
        m_upscale = !m_upscale;
        QSettings settings;
        settings.setValue("scale", m_upscale);
        update();
        break;
    }
    case Qt::Key_D:
        if (sock != -1) {
            QSettings settings;
            // TODO: more clever layout
            const QRect screenGeometry = screen()->availableGeometry();
            QPoint p = position();
            if (p.y() + 10 > screenGeometry.height() - height() * 2) {
                p.setY(0);
                if (p.x() + 10 > screenGeometry.width() - width() * 2) {
                    p.setX(0);
                } else {
                    p.setX(p.x() + width() + 10);
                }
            } else {
                p.setY(p.y() + height() + 10);
            }
            settings.setValue("lastposition", p);
            char tmp = '\0';
            ::write(sock, &tmp, 1);
            ::close(sock);
            sock = -1;
        }
        break;
    case Qt::Key_Up:
        updateScale(1.1);
        break;
    case Qt::Key_Down:
        updateScale(1/1.1);
        break;
    case Qt::Key_PageUp:
        updateScale(2);
        break;
    case Qt::Key_Backspace:
        scale = 1.;
        updateScale(1.);
        break;
    case Qt::Key_PageDown:
        updateScale(0.5);
        break;
    case Qt::Key_Left:
        m_selected = Local;
        update();
        break;
    case Qt::Key_Right:
        m_selected = Remote;
        update();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        save();
        close();
        break;
    default:
        QRasterWindow::keyPressEvent(e);
        return;
    }
}

void Window::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(QRect(0, 0, width(), height()), m_background);

    QRect textRect(0, 0, width(), m_textHeight);
    p.fillRect(textRect, Qt::black);
    p.setPen(Qt::white);

    QString title = m_outputFilename.isEmpty() ? name : m_outputFilename;
    if (!qFuzzyCompare(scale, 1.f)) {
        title += " (" + QString::number(int(scale * 100)) + "%)";
    }
    p.drawText(textRect, Qt::AlignCenter, title);

    int margins = m_outputFilename.isEmpty() ? 0 : 2;

    QSize imgsize(width()/2 - margins * 2, height() - m_textHeight * 2);
    if (!m_localImage.isNull()) {
        if (m_upscale) {
            p.drawImage(margins, m_textHeight, m_localImage.scaled(imgsize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            p.drawImage(margins, m_textHeight, m_localImage.scaled(m_localImage.size() * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    if (!m_remoteImage.isNull()) {
        if (m_upscale) {
            p.drawImage(width()/2 + margins, m_textHeight, m_remoteImage.scaled(imgsize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            imgsize = m_remoteImage.size();
            p.drawImage(width()/2 + margins, m_textHeight, m_remoteImage.scaled(m_remoteImage.size() * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    QRect localRect(0, 20, width() / 2, height() - 20);
    QRect remoteRect(width()/2, 20, width() / 2, height() - 20);

    if (!m_outputFilename.isEmpty()) {
        p.setPen(QPen(Qt::green, 2));
        if (m_selected == Remote) {
            p.drawRect(remoteRect.marginsRemoved(QMargins(1, 1, 1, 1)));
        } else if (m_selected == Local){
            p.drawRect(localRect.marginsRemoved(QMargins(1, 1, 1, 1)));
        }
    }
    QString localText = m_outputFilename.isEmpty() ? "Old" : "Local";
    QString remoteText = m_outputFilename.isEmpty() ? "New" : "Remote";
    if (m_localSize.isValid()) {
        localText += " (" + QString::number(m_localSize.width()) + "x" + QString::number(m_localSize.height()) + ")";
    }
    if (m_remoteSize.isValid()) {
        remoteText += " (" +  QString::number(m_remoteSize.width()) + "x" + QString::number(m_remoteSize.height()) + ")";
    }
    p.setPen(Qt::white);
    localRect = QRect(0, height() - m_textHeight, width() / 2, m_textHeight);
    remoteRect = QRect(width()/2, height() - m_textHeight, width() / 2, m_textHeight);
    p.fillRect(localRect, QColor(0, 0, 0, 192));
    p.drawText(localRect, Qt::AlignHCenter | Qt::AlignBottom, localText);
    p.fillRect(remoteRect, QColor(0, 0, 0, 192));
    p.drawText(remoteRect, Qt::AlignHCenter | Qt::AlignBottom, remoteText);
}

void Window::mousePressEvent(QMouseEvent *e)
{
    m_lastMousePos = e->globalPos();
    if (e->pos().y() < m_textHeight) {
        return;
    }

    if (e->pos().x() > width() / 2) {
        m_selected = Remote;
    } else {
        m_selected = Local;
    }
    update();
    e->ignore();
}

void Window::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->pos().y() < m_textHeight) {
        qApp->exit(6);
        return;
    }

    if (e->pos().x() > width() / 2) {
        m_selected = Remote;
    } else {
        m_selected = Local;
    }
    save();
    exit(0);
}
void Window::mouseMoveEvent(QMouseEvent *e)
{
    if (!e->buttons()) {
        return;
    }
    const QPoint delta = e->globalPos() - m_lastMousePos;
    setPosition(position() + delta);
    m_lastMousePos = e->globalPos();
}

void Window::resizeEvent(QResizeEvent *event)
{
    QWindow::resizeEvent(event);
    QMetaObject::invokeMethod(this, &Window::ensureVisible, Qt::QueuedConnection);
}

void Window::moveEvent(QMoveEvent *e)
{
    QSettings settings;
    settings.setValue("lastposition", e->pos());

    QWindow::moveEvent(e);
    QMetaObject::invokeMethod(this, &Window::ensureVisible, Qt::QueuedConnection);
}

void Window::ensureVisible()
{
    const QRect screenGeometry = screen()->availableGeometry();
    QRect newGeometry = geometry();
    if (geometry().left() > screenGeometry.right() - 50) {
        newGeometry.translate(-50, 0);
    } else if (geometry().right() < screenGeometry.left() + 50) {
        newGeometry.translate(50, 0);
    }
    if (geometry().top() > screenGeometry.bottom() - 50) {
        newGeometry.translate(0, -50);
    } else if (geometry().bottom() < screenGeometry.top() + 50) {
        newGeometry.translate(0, 50);
    }
    setPosition(newGeometry.topLeft());
}

void Window::save()
{
    if (m_outputFilename.isEmpty()) {
        return;
    }

    if (m_selected == None) {
        return;
    }

    QFile::remove(m_outputFilename);
    if (m_selected == Local) {
        QFile::copy(m_localFile, m_outputFilename);
        qDebug() << "Saved to" << m_outputFilename;
    } else if (m_selected == Remote) {
        QFile::copy(m_remoteFile, m_outputFilename);
        qDebug() << "Saved to" << m_outputFilename;
    }
    copied = true;
}

bool Window::loadImage(const QString &filename, QImage *image, QSize *size)
{
    QImageReader reader(filename);
    *size = reader.size();
    if (reader.size().width() > maximumWidth() - m_textHeight || reader.size().height() > maximumHeight() - m_textHeight) {
        QSize newSize = reader.size();
        newSize.scale(maximumWidth() - m_textHeight, maximumHeight() - m_textHeight, Qt::KeepAspectRatio);
        reader.setScaledSize(newSize);
    }
    if (!reader.read(image)) {
        qWarning() << reader.errorString();
        return false;
    }
    return true;
}
