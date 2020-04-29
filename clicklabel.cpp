#include <QDebug>
#include <QMouseEvent>
#include "clicklabel.h"

ClickLabel::ClickLabel(QWidget* parent, Qt::WindowFlags f): QLabel(parent) {
    Q_UNUSED(f)
    init();
}

ClickLabel::ClickLabel(const QString& label, QWidget* parent, Qt::WindowFlags f): QLabel(label, parent) {
    Q_UNUSED(f)
    init();
}

void ClickLabel::init(){
    normPal = this->palette();
    hoverPal = this->palette();
    int h, s, l; QColor col;
    normPal.color(QPalette::Highlight).getHsl(&h,&s,&l);
    col.setHsl(h,s,110);
    //normPal.setColor(this->foregroundRole(), col);
    normPal.setColor(this->foregroundRole(), normPal.color(QPalette::Active,QPalette::Highlight));
    col.setHsl(h,s,140);
    //hoverPal.setColor(this->foregroundRole(),col);
    hoverPal.setColor(this->foregroundRole(), normPal.color(QPalette::Inactive,QPalette::Highlight));
    this->setPalette(normPal);

    this->setMargin(2);
}

void ClickLabel::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton)
        emit clicked();
}

void ClickLabel::enterEvent(QEvent *event) {
    Q_UNUSED(event)
    this->setPalette(hoverPal);
}

void ClickLabel::leaveEvent(QEvent *event) {
    Q_UNUSED(event)
    this->setPalette(normPal);
}
