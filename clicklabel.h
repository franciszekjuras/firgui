#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    explicit ClickLabel(const QString& label,QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

    QPalette normPal;
    QPalette hoverPal;
private:
    void init();

};

#endif // CLICKLABEL_H
