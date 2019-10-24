#include "titlebar.h"
#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QWindow>

TitleBar::TitleBar(QWidget *parent) : QDialog(parent)
{
    setWindowFlag(Qt::Tool, true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tbH = 0;
    setFixedHeight(tbH);
    yPos = 200;
    bottomExtent = 5;
    qApp->installEventFilter(this);
    setContentsMargins(0,0,0,bottomExtent);
}

bool TitleBar::eventFilter(QObject* obj, QEvent* event){
//    if(event->type() == QEvent::Leave && obj == this && isActiveWindow()){
//        parentWidget()->activateWindow();
//    }
//    if(event->type() == QEvent::FocusAboutToChange && (obj->objectName() != QString("TitleBarClassWindow"))){
//        return true;
//    }

    if(event->type() == QEvent::Move && obj == parentWidget()){
        if(tbH <= 0){
            tbH = parentWidget()->geometry().y() - parentWidget()->pos().y();
        }

        setFixedHeight(tbH + bottomExtent);
        move(parentWidget()->geometry().topLeft() + QPoint(yPos,-tbH));
    }

    if(obj == parentWidget() && event->type() == QEvent::WindowStateChange){
        QScreen *screen = parentWidget()->window()->windowHandle()->screen();
        if(screen != nullptr &&  parentWidget()->isMaximized()){
            int maximizedH = parentWidget()->geometry().y() - screen->availableGeometry().y();
            setFixedHeight(maximizedH + bottomExtent);
            move(parentWidget()->geometry().topLeft() + QPoint(yPos,-maximizedH));
        }
        else{
            setFixedHeight(tbH);
        }
    }

    return QDialog::eventFilter(obj, event);
}
