#ifndef GROUPSPECS_H
#define GROUPSPECS_H

#include <QGroupBox>
#include <string>
#include <vector>
#include <QMap>
#include <QString>
#include <QMap>
#include <QThread>
#include <memory>
#include "firker.h"

class QLineEdit;
class QComboBox;
class WaitingSpinnerWidget;

class KernelCalcThread : public QThread{
    Q_OBJECT
signals:
    void calcFinished(FirKer ker);
    void calcFailed();

private:
    std::unique_ptr<FirKer> ker;
public:
    void setKernel(const EqRippleFirKer& ker){
        this->ker = std::make_unique<EqRippleFirKer>(ker);
    }
    void setKernel(const LeastSqFirKer& ker){
        this->ker = std::make_unique<LeastSqFirKer>(ker);
    }
private:
    void run() override {
        if(ker->calc())
            emit calcFinished(*ker);
        else
            emit calcFailed();
    }
};

class GroupSpecs : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupSpecs(QWidget *parent = 0);

private:
    QLineEdit* freqsLineEdit;
    QLineEdit* gainsLineEdit;
    QComboBox* bandCombo;
    WaitingSpinnerWidget* waitSpin;

    std::vector<double> crrKer;
    std::vector<double> crrSrcKer;
    double fpgaSampFreq;
    double unitMult;
    LeastSqFirKer::Window crrWnd;
    KernelCalcThread kerCalcThread;
    KernelCalcThread srcKerCalcThread;
    bool calcRunning;
    bool srcCalcRunning;

    int t, d, s;

    bool isKernelReady;
    bool isFilterReady;
    bool isSrcKernelReady;

    static bool textToDoubles(const std::string& str, std::vector<double>& v);
    void kerCalcFinished(FirKer ker);
    void kerCalcFailed();
    void srcKerCalcFinished(FirKer ker);
    void srcKerCalcFailed();
    void kernelReady(bool en);
    void srcKernelReady(bool en);
    void calcSrcKernel();
    void rebuild();
    void wndChanged(QString wndStr);
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
