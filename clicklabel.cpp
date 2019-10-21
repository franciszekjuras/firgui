#include "clicklabel.h"
#include <QDebug>

ClickLabel::ClickLabel(QWidget* parent, Qt::WindowFlags f): QLabel(parent) {
   init();
}

ClickLabel::ClickLabel(const QString& label, QWidget* parent, Qt::WindowFlags f): QLabel(label, parent) {
    init();
}

void ClickLabel::init(){
    normPal = this->palette();
    hoverPal = this->palette();
    int h, s, l; QColor col;
    normPal.color(QPalette::Highlight).getHsl(&h,&s,&l);
    qDebug() << "lightness" << l;
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
    emit clicked();
}

void ClickLabel::enterEvent(QEvent *event) {
    this->setPalette(hoverPal);
}

void ClickLabel::leaveEvent(QEvent *event) {
    this->setPalette(normPal);
}
