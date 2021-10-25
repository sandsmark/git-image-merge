#include <QGuiApplication>
#include <QDebug>
#include <QSettings>
#include "window.h"

#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fds[2] = { -1, -1 };
    if (argc == 8) {
        // Fork, and then maybe wait for child to quit or request detach
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
            perror("socketpair");
            return -1;
        }
        pid_t p = fork();
        if (p != 0) {
            char tmp;
            read(fds[0], &tmp, 1);
            return tmp;
        }
    }

    QGuiApplication a(argc, argv);
    a.setApplicationName("git-image-merge");
    if (argc < 3) {
        qWarning() << "usage:" << argv[0] << "(local file) (remote file) (output file)";
        return 1;
    }
    Window w;
    if (argc == 4) {
        if (!w.setLocalImage(argv[1])) {
            return 2;
        }
        if (!w.setRemoteImage(argv[2])) {
            return 3;
        }
        w.m_outputFilename = argv[3];
    } else if (argc == 8) {
        w.name = argv[1];
        w.setTitle(argv[1]);
        if (!w.setLocalImage(argv[2])) {
            return 2;
        }
        if (!w.setRemoteImage(argv[5])) {
            return 2;
        }
    } else {
        return 1;
    }
    QSettings settings;
    w.m_upscale = settings.value("scale").toBool();
    w.sock = fds[1];
    if (w.m_upscale) {
        QSize lastSize = settings.value("lastSize").toSize();
        QSize size = w.calcSize();
        if (lastSize.isValid()) {
            size.scale(lastSize, Qt::KeepAspectRatio);
        }
        w.scale = w.scaleFromSize(size);

        QPoint lastPos = settings.value("lastposition").toPoint();
        if (!lastPos.isNull()) {
            w.setPosition(lastPos);
        }
    }
    w.resize(w.calcSize());
    w.show();

    int ret = a.exec();
    if (ret != 0) {
        return ret;
    }
    return w.copied || argc == 8 ? 0 : 5;
}
