#include "xcolor.h"

QColor XColor::changeHslLigthness(const QColor& col, int l){
    if(l < 0) l = 0;
    else if (l > 255) l = 255;
    int h, s, l2;
    col.getHsl(&h,&s,&l2);
    QColor rcol;
    rcol.setHsl(h,s,l);
    return rcol;
}
