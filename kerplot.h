#ifndef PLOT_H
#define PLOT_H

#include <QVector>
#include "qcustomplot.h"
#include "firker.h"

class KerPlot : public QCustomPlot{
    Q_OBJECT

public:
    KerPlot(QWidget* parent = 0);
    //~Plot();
    void setKernel(const FirKer& kernel);
protected:
    QCPRange maxXRange;
    QCPRange maxYRange;

public slots:
    void checkXBounds(const QCPRange& newRange, const QCPRange& oldRange);

};

#endif // PLOT_H
