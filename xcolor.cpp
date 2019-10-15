#include "xcolor.h"
#include <QDebug>

QColor XColor::changeHslLigthness(const QColor& col, int l){
    if(l < 0) l = 0;
    else if (l > 255) l = 255;
    int h, s, l2;
    col.getHsl(&h,&s,&l2);
    QColor rcol;
    rcol.setHsl(h,s,l);
    return rcol;
}


QColor XColor::getSolarizedAccent(const QColor& col){
    if(!col.isValid()){
        qDebug() << "Color not valid. Choosing default color.";
        return blue;
    }
    if(col.hslSaturation()<50){
        qDebug() << "Accent not saturated. Choosing default color.";
        return blue;
    }
    int hue = col.hue();
    if(hue < 9) return red;
    if(hue < 38) return orange;
    if(hue < 65) return yellow;
    if(hue < 150) return green;
    if(hue < 185) return cyan;
    if(hue < 235) return blue;
    if(hue < 285) return violet;
    if(hue < 340) return magenta;
    return red;
    /*
    qDebug() << "yellow" << yellow.lightness() << yellow.hue() << yellow.saturation();
    qDebug() << "orange" << orange.lightness() << orange.hue() << orange.saturation();
    qDebug() << "red" << red.lightness() << red.hue() << red.saturation();
    qDebug() << "magenta" << magenta.lightness() << magenta.hue() << magenta.saturation();
    qDebug() << "violet" << violet.lightness() << violet.hue() << violet.saturation();
    qDebug() << "blue" << blue.lightness() << blue.hue() << blue.saturation();
    qDebug() << "cyan" << cyan.lightness() << cyan.hue() << cyan.saturation();
    qDebug() << "green" << green.lightness() << green.hue() << green.saturation();*/
}
