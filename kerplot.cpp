#include <QVector>
#include <QString>
#include <cmath>
#include "kerplot.h"

KerPlot::KerPlot(QWidget* parent):
    QCustomPlot(parent)
{
    //this->setOpenGl(true);

    this->axisRect()->setRangeDrag(Qt::Horizontal);
    this->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(this->xAxis, SIGNAL(rangeChanged(const QCPRange&, const QCPRange&)),
            this, SLOT(checkXBounds(const QCPRange&, const QCPRange&)));
    this->setEnabled(false);


    this->addGraph();
    this->graph(0)->setPen(QPen(Qt::blue));
    this->xAxis->setLabel(tr("Frequency, kHz"));
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //default values
    setFreq(1.);
    maxGain = 1.;
    plotPoints = 20000;
}

void KerPlot::setKernel(const FirKer &kernel){
    std::vector<double> trns = kernel.transmission(plotPoints);

    transmission = QVector<double>::fromStdVector(trns);
    transmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    double max;
    for(auto v : trns)
        if(v > max) max = v;
    this->maxGain = max;
    this->maxGaindB = 20 * std::log10(max);

    setFreq(kernel.getSampFreq());
    setPlotType(plotType);
}

void KerPlot::clearPlot(){
    transmission = QVector<double>();
    transmissionBode = QVector<double>();
    setFreq(1.);

    this->maxGain = 1.;
    this->maxGaindB = 0.;

    setPlotType(plotType);
    //this->setEnabled(false);
}

void KerPlot::amplitudePlot(){
    this->yAxis->setLabel(tr("Amplitude"));
    this->graph(0)->setData(freqs,transmission,true);
    this->yAxis->setRange(QCPRange(0.,(maxGain*1.02)));
    this->setEnabled(true);
    this->replot();
}

void KerPlot::bodePlot(){
    this->yAxis->setLabel(tr("Attenuation, dB"));
    this->graph(0)->setData(freqs,transmissionBode,true);

    this->yAxis->setRange(QCPRange(-80., maxGaindB+1));

    this->setEnabled(true);
    this->replot();
}

void KerPlot::checkXBounds(const QCPRange& newRange, const QCPRange& oldRange){
    if(newRange.lower < this->xRange.lower || newRange.upper > this->xRange.upper){
        if(newRange.size() == oldRange.size())
            this->xAxis->setRange(oldRange);
        else if(newRange.size() > this->xRange.size()){
            this->xAxis->setRange(this->xRange);
        }
        else if(newRange.lower < this->xRange.lower){
            this->xAxis->setRange(xRange.lower, xRange.lower + newRange.size());
        }
        else{
            this->xAxis->setRange(xRange.upper - newRange.size(), xRange.upper);
        }
    }
}

void KerPlot::setFreq(double freq){
    this->xRange = QCPRange(0., freq/2.);
    this->xAxis->setRange(xRange);

    size_t l = this->transmission.size();

    this->freqs.resize(l);
    const double div = static_cast<double>(l-1);
    for(int i = 0; i < l; ++i){
        this->freqs[i] = static_cast<double>(i) * freq / div / 2.;
    }
}


void KerPlot::setPlotType(const QString& plotType){
    this->plotType = plotType;
    if(plotType == tr("Bode Plot")){
        bodePlot();
    }
    else if(plotType == tr("Amplitude Plot")){
        amplitudePlot();
    }
}

//DataPlot::DataPlot(QWidget* parent):
//    QCustomPlot(parent)
//{
//    this->axisRect()->setRangeDrag(Qt::Horizontal);
//    this->axisRect()->setRangeZoom(Qt::Horizontal);
//    connect(this->xAxis, SIGNAL(rangeChanged(QCPRange,QCPRange)),
//            this, SLOT(checkXBounds(QCPRange,QCPRange)));
//    this->setEnabled(false);
//}

//DataPlot::~DataPlot(){}

//void DataPlot::setData(Measurement measure){
//    this->info = measure.getInfo();
//    this->setEnabled(true);
//}

//QCPRange DataPlot::calcRange(QVector<double> data){
//    if(data.isEmpty())
//        return QCPRange();
//    double min = data.at(0); double max = data.at(0);
//    for(int i = 1; i < data.size(); ++i){
//        if(data.at(i) > max)
//            max = data.at(i);
//        if(data.at(i) < min)
//            min = data.at(i);
//    }
//    return QCPRange(min, max);
//}

//void DataPlot::checkXBounds(const QCPRange& newRange, const QCPRange& oldRange){
//    if(newRange.lower < this->xRange.lower || newRange.upper > this->xRange.upper){
//        if(newRange.size() == oldRange.size())
//            this->xAxis->setRange(oldRange);
//        else if(newRange.size() > this->xRange.size()){
//            this->xAxis->setRange(this->xRange);
//        }
//        else if(newRange.lower < this->xRange.lower){
//            this->xAxis->setRange(xRange.lower, xRange.lower + newRange.size());
//        }
//        else{
//            this->xAxis->setRange(xRange.upper - newRange.size(), xRange.upper);
//        }
//    }
//}

///*--- ABS PLOT ---*/

//AbsPlot::AbsPlot(QWidget* parent):
//    DataPlot(parent)
//{
//    this->addGraph();
//    this->xAxis->setLabel("Frequency, kHz");
//    this->yAxis->setLabel("Amplitude");
//    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//    this->replot();

//}

//AbsPlot::~AbsPlot(){}

//void AbsPlot::setData(Measurement measure){
//    DataPlot::setData(measure);
//    this->data = ArrayOp::abs(measure.getFtData());
//    this->fitFreq();
//    this->graph(0)->setData(this->freq, this->data, true);

//    this->xRange = QCPRange(this->freq.constFirst(), this->freq.constLast());
//    this->xAxis->setRange(xRange);
//    this->yRange = this->calcRange(this->data);
//    this->yAxis->setRange(yRange);
//    this->replot();
//}

//void AbsPlot::fitFreq(){
//    qDebug() << this->data.size();
//    this->freq.resize(this->data.size());
//    for(double i = 0; i < (double)freq.size(); ++i){
//        this->freq[i] = i * this->info.getRatio() / (double)freq.size() / 2 / 1000.0; /*kHz*/
//    }
//}

///*--- COMPLEX PLOT ---*/

//ComplexPlot::ComplexPlot(QWidget* parent):
//    DataPlot(parent)
//{
//    this->addGraph();
//    this->addGraph();
//    this->graph(1)->setPen(QPen(Qt::red));
//    this->xAxis->setLabel("Frequency, kHz");
//    this->yAxis->setLabel("Spectrum");
//    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//    this->replot();

//}

//ComplexPlot::~ComplexPlot(){}

//void ComplexPlot::setData(Measurement measure){
//    DataPlot::setData(measure);
//    this->data = measure.getFtData();
//    //setupPlot
//    this->fitFreq();

//    this->xRange = QCPRange(this->freq.constFirst(), this->freq.constLast());
//    this->xAxis->setRange(xRange);
//    this->yRange = this->calcRange(ArrayOp::abs(this->data));
//    this->yRange.lower = -this->yRange.upper;
//    this->yAxis->setRange(yRange);

//    shiftPhaseAndReplot(0);
//}

//void ComplexPlot::setPhaseShift(int phaseShift){
//    if(this->phaseShift == phaseShift)
//        return;
//    this->shiftPhaseAndReplot(phaseShift);

//}

//void ComplexPlot::shiftPhaseAndReplot(int phaseShift){
//    this->phaseShift = phaseShift;

//    emit valueChangedPhaseShift(this->phaseShift);
//    this->data_ph = ArrayOp::phaseShift(data,(double)this->phaseShift);

//    this->graph(0)->setData(this->freq, ArrayOp::real(this->data_ph), true);
//    this->graph(1)->setData(this->freq, ArrayOp::imag(this->data_ph), true);
//    this->replot();
//}

//void ComplexPlot::fitFreq(){
//    this->freq.resize(this->data.size());
//    for(double i = 0; i < (double)freq.size(); ++i){
//        this->freq[i] = i * this->info.getRatio() / (double)freq.size()/ 2 / 1000.0; /*kHz*/
//    }
//}

///*--- SOURCE PLOT ---*/

//SourcePlot::SourcePlot(QWidget* parent):
//    DataPlot(parent)
//{
//    this->addGraph();
//    this->xAxis->setLabel("Time, ms");
//    this->yAxis->setLabel("Readout");
//    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//    this->replot();

//}

//SourcePlot::~SourcePlot(){}

//void SourcePlot::setData(Measurement measure){
//    DataPlot::setData(measure);
//    this->data = measure.getRawData();
//    this->fitTime();
//    this->graph(0)->setData(this->time, this->data, true);

//    this->xRange = QCPRange(this->time.constFirst(), this->time.constLast());
//    this->xAxis->setRange(xRange);
//    this->yRange = this->calcRange(this->data);
//    this->yAxis->setRange(yRange);

//    this->replot();
//}

//void SourcePlot::fitTime(){
//    this->time.resize(this->data.size());
//    for(double i = 0; i < (double)time.size(); ++i){
//        this->time[i] = i / this->info.getRatio() * 1000.0; /*ms*/
//    }
//}
