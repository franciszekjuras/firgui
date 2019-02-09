#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QDebug>
#include "window.h"

int main(int argc, char *argv[])
{
    QApplication::setStyle("cleanlooks");
    QApplication app(argc, argv);
    Window window;
    //window.setStyleSheet("QGroupBox {border: 1px solid gray; border-radius: 9px; margin-top: 0.5em;}"
     //                    "QGroupBox::title {subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px;}");
    QRect wRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,window.size(),app.desktop()->availableGeometry());
    qDebug() << wRect;
    wRect.adjust(0, -50, 0, -50);
    qDebug() << wRect;
    window.setGeometry(wRect);
    window.show();
    return app.exec();
}
