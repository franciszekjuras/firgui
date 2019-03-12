#ifndef GROUPCONFIG_H
#define GROUPCONFIG_H

#include <QGroupBox>
#include <QString>
#include <QMap>
#include "groupconfig.h"
#include "bitstreamspecs.h"

class QComboBox;

class GroupConfig : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupConfig(QWidget *parent = 0);

    void init();

private:
    QComboBox* bitMainCombo;
    QString bitMainStr;
    QComboBox* bitSpecCombo;
    double fpgaSampFreq;
    QMap<QString, QMap<QString,BitstreamSpecs> > bitMap;
    BitstreamSpecs crrBitstream;

    void bitMainComboChanged(QString mainStr);
    void bitSpecComboChanged(QString specStr);
    void updateBitSpecCombo(QString mainStr);

    void onLoadButton();

signals:
    void bitstreamSelected(QMap<QString, int> specs);
    void fpgaSampFreqChanged(double freq);

    void enableLoad(bool en);
    void reqLoad(BitstreamSpecs bitSpecs);

public slots:
};

//"  [-tm | --time-multiplexing] Time multiplexing rank\n"
//"  [-cs | --coef-size] Single coefficient byte-size\n"
//"  [-sr | --src-rank] Sampling rate conversion kernel rank\n"
//"  [-fr | --fir-rank] FIR kernel rank\n"
//"  [-sb | --src-blocks] Sampling rate conversion blocks per one conversion\n"
//"  [-fb | --fir-blocks] FIR blocks\n"
//"  [-sp | --src-precision] Sampling rate conversion kernel fixed point precision\n"
//"  [-fp | --fir-precision] FIR fixed point precision\n"
//"  --load:      Load kernel\n"
//"  --enable:    Enable filter\n"
//"  --disable:   Disable filter\n"
//"  --zero:      Set all coefficients to 0\n"
//"  --info:      Show info\n"

#endif // GROUPCONFIG_H
