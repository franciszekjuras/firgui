#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QDebug>
#include <QFile>
#include "window.h"
#include <libssh/libssh.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":data/icon.png"));

    qDebug() << "libssh: " << ssh_version(0);

    Window window;
    QRect wRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,window.size(),app.desktop()->availableGeometry());
    wRect.adjust(0, -50, 0, -50);
    window.setGeometry(wRect);
    window.show();
    return app.exec();
}
