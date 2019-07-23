
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMainWindow>

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);

private:
    void setTooltipsVisible(bool v);
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif
