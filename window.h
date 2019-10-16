
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMainWindow>

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(bool darkTheme = false, QWidget *parent = nullptr);

    void setDarkTheme(bool darkTheme);

    int defaultFontSize;

private:
    bool isDarkTheme;
    void setTooltipsVisible(bool v);
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif
