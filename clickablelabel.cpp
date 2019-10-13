#include "clickablelabel.h"
#include <QDebug>

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f): QLabel(parent) {
   init();
}

ClickableLabel::ClickableLabel(const QString& label, QWidget* parent, Qt::WindowFlags f): QLabel(label, parent) {
    init();
}

void ClickableLabel::init(){
    int h, s, l; QColor col;
    normPal.color(QPalette::Highlight).getHsl(&h,&s,&l);
    qDebug() << "lightness" << l;
    col.setHsl(h,s,110);
    normPal.setColor(this->foregroundRole(), col);
    col.setHsl(h,s,140);
    hoverPal.setColor(this->foregroundRole(),col);
    this->setPalette(normPal);

    this->setMargin(4);
}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
    emit clicked();
}

void ClickableLabel::enterEvent(QEvent *event) {
    this->setPalette(hoverPal);
}

void ClickableLabel::leaveEvent(QEvent *event) {
    this->setPalette(normPal);
}
