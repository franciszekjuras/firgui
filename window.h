
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QTextBrowser>
#include "titlebar.h"

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(bool darkTheme = false, QWidget *parent = nullptr);

    void toogleFullscreen();
    void setDarkTheme(bool darkTheme);

    int defaultFontSize;

private:
    bool wasMaximized;
    bool isDarkTheme;
    void showHelp(const QString& anchor);
    void showGetStarted();
    bool eventFilter(QObject* obj, QEvent* event);

    TitleBar* titleBar;
    QTextBrowser* helpBrowser;
};

#endif
