#include <QGuiApplication>
#include <QDebug>
#include "window.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
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
    w.resize(w.calcSize());
    w.show();

    int ret = a.exec();
    if (ret != 0) {
        return ret;
    }
    return w.copied || argc == 8 ? 0 : 5;
}
