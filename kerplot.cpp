#include <QVector>
#include <QString>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <cmath>
#include <algorithm>
#include <memory>
#include <cassert>
#include "kerplot.h"

KerPlot::KerPlot(QWidget* parent):
    QCustomPlot(parent)
{
    //this->setOpenGl(true);

    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, &KerPlot::cntSetKernel);

    this->axisRect()->setRangeDrag(Qt::Horizontal);
    this->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(this->xAxis, SIGNAL(rangeChanged(const QCPRange&, const QCPRange&)),
            this, SLOT(checkXBounds(const QCPRange&, const QCPRange&)));
    this->setEnabled(false);


    this->addGraph();
    this->graph(0)->setPen(QPen(QColor(57, 106, 177)));
    this->addGraph();
    this->graph(1)->setPen(QPen(QColor(218, 124, 48)));
    //this->setNotAntialiasedElements(QCP::aeAll);
    this->graph(0)->setAdaptiveSampling(false);
    this->graph(1)->setAdaptiveSampling(false);

    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //default values
    setFreqs(1., 1, 0);
    kerMaxGain = 1.;
    srcKerMaxGain = 1.;
    plotPoints = 20000;
    srcPlotPoints = 10000;
}

void KerPlot::setKernel(std::shared_ptr<const FirKer> kernel){
    QFuture<std::vector<double> > fut = QtConcurrent::run([=](){return kernel->transmission(plotPoints);});
    kerTransWatch.setFuture(fut);
    plotClearedMeanwhile = false;
}

void KerPlot::cntSetKernel(){
    if(plotClearedMeanwhile) return;
    std::vector<double> trns = kerTransWatch.future().result();
    //std::vector<double> trns = kernel->transmission(plotPoints);
    if(inverseBand)
        std::reverse(trns.begin(), trns.end());

    transmission = QVector<double>::fromStdVector(trns);
    transmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    if(freqs.size()-1 != plotPoints)
        updateFreqs();

    double max;
    for(auto v : trns)
        if(v > max) max = v;
    kerMaxGain = max;

    setPlotType(plotType); //this will replot
}

void KerPlot::setSrcKernel(std::shared_ptr<const FirKer> kernel){
    std::vector<double> trns = kernel->transmission(srcPlotPoints);

    srcTransmission = QVector<double>::fromStdVector(trns);
    srcTransmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    if(srcFreqs.size()-1 != srcPlotPoints)
        updateSrcFreqs();

    double max;
    for(auto v : trns)
        if(v > max) max = v;
    srcKerMaxGain = max;

    setPlotType(plotType);
}

void KerPlot::clearKernel(){
    plotClearedMeanwhile = true;
    transmission.clear();
    transmissionBode.clear();
    freqs.clear();
    kerMaxGain = 1.;
    setPlotType(plotType); //this will replot
}

void KerPlot::clearSrcKernel(){
    srcTransmission.clear();
    srcTransmissionBode.clear();
    srcFreqs.clear();
    srcKerMaxGain = 1.;
    setPlotType(plotType); //this will replot
}

void KerPlot::resetPlot(double freq, int t, int band){
    setFreqs(freq, t, band);
    clearKernel();
    clearSrcKernel();
}

void KerPlot::amplitudePlot(){
    this->yAxis->setLabel(tr("Amplitude"));

    assert(freqs.size() == transmission.size());
    this->graph(0)->setData(freqs,transmission,true);
    assert(srcFreqs.size() == srcTransmission.size());
    this->graph(1)->setData(srcFreqs,srcTransmission,true);

    double maxGain = std::max(srcKerMaxGain, kerMaxGain);
    this->yAxis->setRange(QCPRange(0.,(maxGain*1.02)));

    this->setEnabled(true);
    this->replot();
}

void KerPlot::bodePlot(){
    this->yAxis->setLabel(tr("Attenuation, dB"));

    assert(freqs.size() == transmissionBode.size());
    this->graph(0)->setData(freqs,transmissionBode,true);

    assert(srcFreqs.size() == srcTransmissionBode.size());
    this->graph(1)->setData(srcFreqs,srcTransmissionBode,true);

    double maxGaindB = 20 * std::log10(std::max(srcKerMaxGain, kerMaxGain));
    this->yAxis->setRange(QCPRange(-100., maxGaindB+1));

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

void KerPlot::setFreqs(double freq, int t, int band){
    nqFreq = freq/2.;
    inverseBand = ((band%2) == 1);
    double dT = static_cast<double>(t);
    double dBand = static_cast<double>(band);

    lBandLimit = dBand*freq/2./dT;
    rBandLimit = (dBand+1)*freq/2./dT;

    if(rBandLimit/2. > 5000.){
        rBandLimit /= 1000.; lBandLimit /= 1000.; nqFreq/=1000.;
        this->xAxis->setLabel(tr("Frequency, MHz"));
    }else
        this->xAxis->setLabel(tr("Frequency, kHz"));

    this->xRange = QCPRange(lBandLimit, rBandLimit);
    this->xAxis->setRange(xRange);

}

void KerPlot::updateFreqs(){
    size_t l = plotPoints + 1;
    this->freqs.resize(l);
    double div = static_cast<double>(l - 1);
    double step = (rBandLimit - lBandLimit)/div;
    for(int i = 0; i < l; ++i){
        this->freqs[i] = lBandLimit + static_cast<double>(i) * step;
    }
}

void KerPlot::updateSrcFreqs(){
    size_t l = srcPlotPoints + 1;
    this->srcFreqs.resize(l);
    double div = static_cast<double>(l - 1);
    double step = nqFreq/div;
    for(int i = 0; i < l; ++i){
        this->srcFreqs[i] = static_cast<double>(i) * step;
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

void KerPlot::toggleSrcTransPlot(bool toggle){
    this->graph(1)->setVisible(toggle);
    replot();
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
