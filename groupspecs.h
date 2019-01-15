#ifndef GROUPSPECS_H
#define GROUPSPECS_H

#include <QGroupBox>
#include <string>
#include <vector>
#include <QMap>
#include <QString>
#include "firker.h"

class QLineEdit;

class GroupSpecs : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupSpecs(QWidget *parent = 0);

private:
    QLineEdit* freqsLineEdit;
    QLineEdit* gainsLineEdit;
    std::vector<double> crrKer;
    double fpgaSampFreq;
    double kerSampFreq;
    int kerRank;
    bool calcEn;

    void setCalcEn(bool en){calcEn = en;}
    static bool textToDoubles(const std::string& str, std::vector<double>& v);

public slots:
    void calculateKernel();
    void bitstreamChanged(QMap<QString, int> specs);
    void bitstreamLoaded();
    void setFpgaSampFreq(double freq);

signals:
    void kernelChanged(const FirKer& ker);
    void enableCalcButton(bool en);
    void enableSetButton(bool en);
    void reqClearPlot();


};

#endif // GROUPSPECS_H
