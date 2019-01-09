#include <QVector>
#include "kerplot.h"

KerPlot::KerPlot(QWidget* parent):
    QCustomPlot(parent)
{

}

void KerPlot::setKernel(const FirKernel& kernel){

}

void KerPlot::checkXBounds(const QCPRange& newRange, const QCPRange& oldRange){

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
