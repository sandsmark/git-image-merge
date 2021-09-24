#ifndef WINDOW_H
#define WINDOW_H

#include <QRasterWindow>
#include <QDebug>
#include <QImage>

class Window : public QRasterWindow
{
    Q_OBJECT
public:
    Window();

    QString m_outputFilename;
    QString name;

    bool copied = false;

    bool setLocalImage(const QString &filename);
    bool setRemoteImage(const QString &filename);
    QSize calcSize() {
        qreal w = qMax(m_remoteImage.width(), m_localImage.width()) * 2.;
        qreal h = qMax(m_remoteImage.height(), m_localImage.height());
        QSize s(w * scale, h * scale);
//        s *= scale;
        if (s.width() < minimumWidth() || s.height() < minimumHeight()) {
            s = s.scaled(minimumSize(), Qt::KeepAspectRatioByExpanding);
        }
        if (s.width() > maximumWidth() || s.height() > maximumHeight() - 20) {
            s = s.scaled(maximumWidth(), maximumHeight() - 20, Qt::KeepAspectRatio);
        }
        scale = float(s.width()) / w;
        return QSize(s.width(), s.height() + 20);
    }
    void updateSize(float delta) {
        scale = qMax(delta * scale, 0.00001f);
        QRect r = geometry();
        QPoint center = r.center();
        r.setSize(calcSize());
        r.moveCenter(center);
        setGeometry(r);
    }

protected:
    void keyPressEvent(QKeyEvent *) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

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
    float scale = 1.;
    bool m_upscale = false;
};

#endif // WINDOW_H
