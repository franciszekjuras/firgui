#ifndef PLOT_H
#define PLOT_H

#include <QVector>
#include <QString>
#include "qcustomplot.h"
#include "firker.h"

class KerPlot : public QCustomPlot{
    Q_OBJECT

public:
    KerPlot(QWidget* parent = 0);
    //~Plot();
protected:
    QCPRange xRange;
    double maxGain;
    double maxGaindB;
    QString plotType;
    int plotPoints;

    QVector<double> transmission;
    QVector<double> transmissionBode;
    QVector<double> freqs;

    void amplitudePlot();
    void bodePlot();
    void setFreq(double freq);

public slots:
    void checkXBounds(const QCPRange& newRange, const QCPRange& oldRange);
    void setKernel(const FirKer& kernel);
    void setPlotType(const QString& plotType);

};

#endif // PLOT_H
