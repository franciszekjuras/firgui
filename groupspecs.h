#ifndef GROUPSPECS_H
#define GROUPSPECS_H

#include <QGroupBox>
#include <string>
#include <vector>
#include <QMap>
#include <QString>
#include <QMap>
#include <QThread>
#include <QFutureWatcher>
#include <memory>
#include "firker.h"
#include "boolmapwatcher.h"

//Q_DECLARE_METATYPE(std::shared_ptr<FirKer>)

class QLineEdit;
class QComboBox;
class WaitingSpinnerWidget;

//class KernelCalcThread : public QThread{
//    Q_OBJECT
//signals:
//    void calcFinished(std::shared_ptr<FirKer> ker);
//private:
//    std::shared_ptr<FirKer> ker;
//public:
//    void setKernel(std::shared_ptr<FirKer> ker){
//        this->ker = ker;
//    }
//private:
//    void run() override {
//        ker->calc();
//        emit calcFinished(ker);
//    }
//};

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
    //KernelCalcThread kerCalcThread;
    //KernelCalcThread srcKerCalcThread;

    QFutureWatcher<std::shared_ptr<FirKer> > kerCalcWatch;
    QFutureWatcher<std::shared_ptr<FirKer> > srcKerCalcWatch;

    int t, d, s;

    bool isKernelReady;
    bool isFilterReady;
    bool isSrcKernelReady;
    bool isSrcKernelLoaded;
    bool pendCalculateKernel;
    bool pendCalcSrcKernel;
    bool kerLocked;
    bool srcKerLocked;
    BoolMapOr spinWatch;
    bool middleBandsEn;

    static bool textToDoubles(const std::string& str, std::vector<double>& v);
    void kerCalcFinished();
    void srcKerCalcFinished();
    void kernelReady(bool en);
    void srcKernelReady(bool en);
    void calcSrcKernel();
    void rebuild();
    void wndChanged(QString wndStr);
    void bandChanged(int band);
    void unitChanged(QString unit);
    void showHelp();
    int currentBand();

public slots:
    void calculateKernel();
    void setKernels();
    void bitstreamChanged(QMap<QString, int> specs);
    void bitstreamLoaded(QMap<QString, int> specs);
    void filterReady(bool en);
    void setFpgaSampFreq(double freq);
    void handleConnect(bool is);

signals:
    void kernelChanged(std::shared_ptr<const FirKer> ker);
    void kernelClear();
    void srcKernelChanged(std::shared_ptr<const FirKer> ker);
    void srcKernelClear();
    void enableCalcButton(bool en);
    void enableSetButton(bool en);
    void resetPlot(double freq, int t, int band);
    void reqLoadSrcKernel(std::vector<double> crrSrcKer);
    void reqLoadKernel(std::vector<double> crrKer);


};

#endif // GROUPSPECS_H
