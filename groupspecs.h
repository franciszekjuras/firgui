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

class QLineEdit;
class QComboBox;
class WaitingSpinnerWidget;
#ifdef _WIN32
class GroupSpecs : public QWidget
#else
class GroupSpecs : public QGroupBox
#endif
{
    Q_OBJECT
public:
    explicit GroupSpecs(QWidget *parent = nullptr);

private:
    QLineEdit* freqsLineEdit;
    QLineEdit* gainsLineEdit;
    QComboBox* bandCombo;
    WaitingSpinnerWidget* waitSpin;

    std::vector<double> crrKer;
    std::vector<double> crrSrcKer;
    double fpgaSamplingFreq;
    double unitMultiplier;
    LeastSqFirKer::Window currentWindow;

    QFutureWatcher<std::shared_ptr<FirKer> > calculateKernelWatch;
    QFutureWatcher<std::shared_ptr<FirKer> > calculateSrcKernelWatch;

    int t, d, s;

    double roiL, roiR;

    bool isKernelReady;
    bool isFilterReady;
    bool isSrcKernelReady;
    bool isSrcKernelLoaded;
    bool pendingCalculateKernel;
    bool pendingCalculateSrcKernel;
    BoolMapOr spinWatch;
    bool middleBandsEn;

    static bool textToDoubles(const std::string& str, std::vector<double>& v);
    void calculateKernelFinished();
    void calculateSrcKernelFinished();
    void kernelReady(bool en);
    void srcKernelReady(bool en);
    void calculateSrcKernel();
    void rebuild();
    void windowChanged(QString windowStr);
    void bandChanged(int band);
    void unitChanged(QString unit);
    bool isSpecValid(const std::vector<double>& freqs, const std::vector<double>& gains);
    int currentBand();

    bool eventFilter(QObject* obj, QEvent* event);

public slots:
    void calculateKernel();
    void setKernels();
    void bitstreamChanged(QMap<QString, int> specs);
    void bitstreamLoaded(QMap<QString, int> specs);
    void filterReady(bool en);
    void setfpgaSamplingFreq(double freq);
    void handleConnect(bool is);
    void setSpec(QVector<double> freqs, QVector<double> gains);

signals:
    void textSpecChanged(QVector<double> freqs, QVector<double> gains);
    void kernelChanged(std::shared_ptr<const FirKer> ker, double roiL, double roiR);
//    void kernelClear();
    void srcKernelChanged(std::shared_ptr<const FirKer> ker);
//    void srcKernelClear();
    void enableCalculateButton(bool en);
    void enableSetButton(bool en);
    void resetPlot(double freq, int t, int band);
    void reqLoadSrcKernel(std::vector<double> crrSrcKer);
    void reqLoadKernel(std::vector<double> crrKer);
    void showHelp();

    void updatePalette();
};

#endif // GROUPSPECS_H
