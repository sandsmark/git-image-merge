#ifndef WINDOW_H
#define WINDOW_H

#include <QRasterWindow>
#include <QDebug>
#include <QImage>
#include <QSettings>

class Window : public QRasterWindow
{
    Q_OBJECT
public:
    Window();
    ~Window();

    QString m_outputFilename;
    QString name;

    int sock = -1;
    bool copied = false;

    bool setLocalImage(const QString &filename);
    bool setRemoteImage(const QString &filename);

    float scale = 1.;
    bool m_upscale = false;

    float scaleFromSize(const QSize &size) {
        qreal w = qMax(m_remoteImage.width(), m_localImage.width()) * 2.;
        return size.width() / w;
    }

    QSize calcSize() {
        qreal w = qMax(m_remoteImage.width(), m_localImage.width()) * 2.;
        qreal h = qMax(m_remoteImage.height(), m_localImage.height());
        QSize s(w * scale, h * scale);
        if (s.width() < minimumWidth() || s.height() < minimumHeight()) {
            s = s.scaled(minimumSize(), Qt::KeepAspectRatioByExpanding);
        }
        if (s.width() > maximumWidth() || s.height() > maximumHeight() - 20) {
            s = s.scaled(maximumWidth(), maximumHeight() - 20, Qt::KeepAspectRatio);
        }
        scale = float(s.width()) / w;
        return QSize(s.width(), s.height() + m_textHeight * 2);
    }

    void updateScale(float delta) {
        setScale(scale * delta);
        QSettings settings;
        settings.setValue("lastSize", m_preferredSize);
    }

    void setScale(float scale_) {
        scale = qMax(scale_, 0.00001f);
        QRect r = geometry();
        QPoint center = r.center();
        m_preferredSize = calcSize();
        r.setSize(m_preferredSize);
        r.moveCenter(center);
        setGeometry(r);
    }

public slots:
    void ensureVisible();

protected:
    void keyPressEvent(QKeyEvent *) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void moveEvent(QMoveEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    enum Which {
        Local,
        Remote,
        None
    };
    void save();
    bool loadImage(const QString &filename, QImage *image, QSize *size);

    Which m_selected = None;
    QImage m_localImage;
    QString m_localFile;
    QImage m_remoteImage;
    QString m_remoteFile;
    QPixmap m_background;
    QSize m_localSize;
    QSize m_remoteSize;

    QSize m_preferredSize;

    int m_textHeight = 20;

    QPoint m_lastMousePos;
};

#endif // WINDOW_H
