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

#endif // GROUPCONFIG_H
