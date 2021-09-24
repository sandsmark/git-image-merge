#include "window.h"
#include <QGuiApplication>
#include <QKeyEvent>
#include <QImageReader>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QScreen>

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
        return;
    case Qt::Key_S:
        m_upscale = !m_upscale;
        update();
        return;
    case Qt::Key_Up:
        updateSize(1.1);
        return;
    case Qt::Key_Down:
        updateSize(1/1.1);
        return;
    case Qt::Key_PageUp:
        updateSize(2);
        return;
    case Qt::Key_Backspace:
        scale = 1.;
        updateSize(1.);
        return;
    case Qt::Key_PageDown:
        updateSize(0.5);
        return;
    case Qt::Key_Left:
        m_selected = Local;
        update();
        return;
    case Qt::Key_Right:
        m_selected = Remote;
        update();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        save();
        close();
        return;
    default:
        break;
    }


    QRasterWindow::keyPressEvent(e);
}

void Window::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(QRect(0, 0, width(), height()), m_background);

    QRect textRect(0, 0, width(), 20);
    p.fillRect(textRect, Qt::black);
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setPixelSize(15);
    f.setBold(true);
    p.setFont(f);
    p.drawText(textRect, Qt::AlignCenter, m_outputFilename.isEmpty() ? name : m_outputFilename);

    QSize imgsize(width()/2 - 4, height() - 20 - 4);
    if (!m_localImage.isNull()) {
        if (m_upscale) {
            p.drawImage(2, 22, m_localImage.scaled(imgsize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            p.drawImage(2, 22, m_localImage);
        }
    }
    if (!m_remoteImage.isNull()) {
        if (m_upscale) {
            p.drawImage(width()/2 + 2, 22, m_remoteImage.scaled(imgsize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            p.drawImage(width()/2 + 2, 22, m_remoteImage);
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
    localRect = p.boundingRect(localRect, Qt::AlignHCenter | Qt::AlignBottom, localText);
    remoteRect = p.boundingRect(remoteRect, Qt::AlignHCenter | Qt::AlignBottom, remoteText);
    p.fillRect(localRect.marginsAdded(QMargins(2, 2, 2, 2)), QColor(0, 0, 0, 100));
    p.drawText(localRect, Qt::AlignHCenter | Qt::AlignBottom, localText);
    p.fillRect(remoteRect.marginsAdded(QMargins(2, 2, 2, 2)), QColor(0, 0, 0, 100));
    p.drawText(remoteRect, Qt::AlignHCenter | Qt::AlignBottom, remoteText);
}

void Window::mousePressEvent(QMouseEvent *e)
{
    if (e->pos().y() < 20) {
        qApp->exit(6);
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
    if (e->pos().x() > width() / 2) {
        m_selected = Remote;
    } else {
        m_selected = Local;
    }
    save();
    exit(0);
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
    if (reader.size().width() > maximumWidth() - 20 || reader.size().height() > maximumHeight() - 20) {
        QSize newSize = reader.size();
        newSize.scale(maximumWidth() - 20, maximumHeight() - 20, Qt::KeepAspectRatio);
        reader.setScaledSize(newSize);
    }
    if (!reader.read(image)) {
        qWarning() << reader.errorString();
        return false;
    }
    return true;
}
