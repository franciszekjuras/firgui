#ifndef PLOT_H
#define PLOT_H

#include <QVector>
#include <QString>
#include "qcustomplot.h"
#include "firker.h"

#define KER_PLOT_POINTS 10000
#define SRCKER_PLOT_POINTS 10000

class KerPlot : public QCustomPlot{
    Q_OBJECT

public:
    KerPlot(QWidget* parent = 0);
    //~Plot();
protected:
    QCPRange xRange;
    double kerMaxGain;
    double srcKerMaxGain;
    QString plotType;
    int plotPoints;
    int srcPlotPoints;

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

    void amplitudePlot();
    void bodePlot();
    void setFreqs(double freq, int t, int band);
    void updateFreqs();
    void updateSrcFreqs();

public slots:
    void checkXBounds(const QCPRange& newRange, const QCPRange& oldRange);
    void setKernel(const FirKer& kernel);
    void setSrcKernel(const FirKer& kernel);
    void setPlotType(const QString& plotType);
    void resetPlot(double freq, int t, int band);
    void clearKernel();
    void clearSrcKernel();
    void toggleSrcTransPlot(bool);

};

#endif // PLOT_H
