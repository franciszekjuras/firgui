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
    explicit GroupConfig(QWidget *parent = nullptr);

    void init();

private:
    QComboBox* bandwidthCombo;
    QString bitMainStr;
    QComboBox* rankCombo;
    double fpgaSamplingFreq;
    QMap<QString, QMap<QString,BitstreamSpecs> > bitMap;
    BitstreamSpecs crrBitstream;

    void bandwidthComboChanged(QString mainStr);
    void rankComboChanged(QString specStr);
    void updateRankCombo(QString mainStr);

    void onLoadButton();

signals:
    void bitstreamSelected(QMap<QString, int> specs);
    void fpgaSamplingFreqChanged(double freq);

    void enableLoad(bool en);
    void reqLoad(BitstreamSpecs bitSpecs);    
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
