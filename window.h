
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QTextBrowser>

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
    void showHelp(const QString& anchor);
    void showGetStarted();
    bool eventFilter(QObject* obj, QEvent* event);

    QTextBrowser* helpBrowser;
};

#endif
