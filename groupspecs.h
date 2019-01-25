#ifndef GROUPSPECS_H
#define GROUPSPECS_H

#include <QGroupBox>
#include <string>
#include <vector>
#include <QMap>
#include <QString>
#include "firker.h"

class QLineEdit;
class QComboBox;

class GroupSpecs : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupSpecs(QWidget *parent = 0);

private:
    QLineEdit* freqsLineEdit;
    QLineEdit* gainsLineEdit;
    QComboBox* bandCombo;
    std::vector<double> crrKer;
    std::vector<double> crrSrcKer;
    double fpgaSampFreq;

    int t, d, s;

    bool isKernelReady;
    bool isFilterReady;
    bool isSrcKernelReady;

    static bool textToDoubles(const std::string& str, std::vector<double>& v);
    void kernelReady(bool en);
    void srcKernelReady(bool en){isSrcKernelReady = en;}
    void calcSrcKernel();
    void clearBandsCombo();
    void rebuild();
    void bandChanged(int band);
    void unitChanged(QString unit);

public slots:
    void calculateKernel();
    void bitstreamChanged(QMap<QString, int> specs);
    void bitstreamLoaded(QMap<QString, int> specs);
    void filterReady(bool en);
    void setFpgaSampFreq(double freq);

signals:
    void kernelChanged(const FirKer& ker);
    void kernelClear();
    void srcKernelChanged(const FirKer& ker);
    void srcKernelClear();
    void enableCalcButton(bool en);
    void enableSetButton(bool en);
    void resetPlot(double freq, int t, int band);


};

#endif // GROUPSPECS_H
