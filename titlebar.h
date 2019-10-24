#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QDialog>
#include <QEvent>

class TitleBar : public QDialog
{
    Q_OBJECT
public:
    TitleBar(QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject* obj, QEvent* event);
    int tbH;
    int yPos;
    int bottomExtent;

signals:

public slots:
};

#endif // TITLEBAR_H
