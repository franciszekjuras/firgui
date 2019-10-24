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

QColor XColor::changeHslSaturation(const QColor& col, int s){
    if(s < 0) s = 0;
    else if (s > 255) s = 255;
    int h, s2, l;
    col.getHsl(&h,&s2,&l);
    QColor rcol;
    rcol.setHsl(h,s,l);
    return rcol;
}

QColor XColor::getSolarizedAccent(const QColor& col){
    if(!col.isValid()){
        qWarning() << "Color not valid. Choosing default color.";
        return blue;
    }
    if(col.hslSaturation()<50){
        qInfo() << "Accent not saturated. Choosing default color.";
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
}
