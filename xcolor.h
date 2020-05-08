#ifndef XCOLOR_H
#define XCOLOR_H
#include <QColor>

namespace XColor {

QColor changeHslLigthness(const QColor& col, int l);
QColor changeHslSaturation(const QColor& col, int s);
QColor getSolarizedAccent(const QColor& col);

const QColor base03 = QColor("#212121");
const QColor base02 = QColor(7, 54, 66);
const QColor base01 = QColor(88, 111, 117);
const QColor base00 = QColor(101, 123, 131);
const QColor base0 = QColor(131, 148, 150);
const QColor base1 = QColor(147, 161, 161);
const QColor base2 = QColor(238, 232, 213);
const QColor base3 = QColor("#ffffff");
const QColor yellow = QColor(181, 137, 0);
const QColor orange = QColor(203, 75, 22);
const QColor red = QColor(220, 50, 47);
const QColor magenta = QColor(211, 54, 130);
const QColor violet = QColor(108, 113, 196);
const QColor blue = QColor(38, 139, 210);
const QColor cyan = QColor(42, 161, 152);
const QColor green = QColor(133, 153, 0);

}



#endif // XCOLOR_H
