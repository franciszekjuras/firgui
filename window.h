
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
    Window(QWidget *parent = nullptr);

    void toggleFullscreen();
    void setupTheme(bool darkTheme);

    int defaultFontSize;

protected:
    bool wasMaximized;
    void showHelp(const QString& anchor);
    void showGetStarted();
    virtual bool eventFilter(QObject* obj, QEvent* event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    TitleBar* titleBar;
    QTextBrowser* helpBrowser;
};

#endif
