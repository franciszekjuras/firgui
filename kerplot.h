#ifndef PLOT_H
#define PLOT_H

#include <QVector>
#include <QString>
#include <QFutureWatcher>
#include <memory>
#include "qcustomplot.h"
#include "firker.h"
#include "waitingspinnerwidget.h"
#include "boolmapwatcher.h"

class KerPlot : public QCustomPlot{
    Q_OBJECT

public:
    KerPlot(QWidget* parent = nullptr);
    //~Plot();

    QSize sizeHint() const;
protected:
    QCPRange xRange;
    double kerMaxGain;
    double srcKerMaxGain;
    QString plotType;
    int srcPlotDiv;
    int plotDiv;
    int plotDivScale;
    int t;
    int band;

    //this values are for display only
    bool inverseBand;
    double lBandLimit;
    double rBandLimit;
    double nqFreq;

    QVector<double> transmission;
    QVector<double> transmissionBode;
    QVector<double> srcTransmission;
    QVector<double> srcTransmissionBode;
    QVector<double> freqs;
    QVector<double> srcFreqs;

    QFutureWatcher<std::vector<double> > kerTransWatch;
    QFutureWatcher<std::vector<double> > srcKerTransWatch;
    bool plotClearedMeanwhile;
    bool srcPlotClearedMeanwhile;
    WaitingSpinnerWidget* waitSpin;    
    BoolMapOr spinWatch;

    void amplitudePlot();
    void bodePlot();
    void setFreqs(double freq, int t, int band);
    void updateFreqs();
    void updateSrcFreqs();
    void totalAmplTrans();
    void totalBodeTrans();
    void clearTotalTrans();

    void cntSetKernel();
    void cntSetSrcKernel();

public slots:
    void checkXBounds(const QCPRange& newRange);
    void setKernel(std::shared_ptr<const FirKer> kernel);
    void setSrcKernel(std::shared_ptr<const FirKer> kernel);
    void setPlotType(const QString& plotType);
    void resetPlot(double freq, int t, int band);
    void clearKernel();
    void clearSrcKernel();
    void toggleTotalTransPlot(bool);
    void toggleSrcTransPlot(bool);
    void toggleFirTransPlot(bool);

};

#endif // PLOT_H
