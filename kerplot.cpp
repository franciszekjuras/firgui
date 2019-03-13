#include "kerplot.h"
#include <QVector>
#include <QString>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <cmath>
#include <algorithm>
#include <memory>
#include <cassert>
#include "waitingspinnerwidget.h"
#include "boolmapwatcher.h"


KerPlot::KerPlot(QWidget* parent):
    QCustomPlot(parent)
{
    //this->setOpenGl(true);
    //setNotAntialiasedElement(QCP::aeAxes);
    //setNotAntialiasedElement(QCP::aeGrid);

//    this->yAxis->setLabelColor(QColor(76,76,76));
//    this->xAxis->setLabelColor(QColor(76,76,76));

    waitSpin = new WaitingSpinnerWidget(this, true, true);

    int sizeMult = 2;
    waitSpin->setRoundness(70.0);
    waitSpin->setMinimumTrailOpacity(50.0);
    waitSpin->setTrailFadePercentage(70.0);
    waitSpin->setNumberOfLines(10);
    waitSpin->setLineLength(6 * sizeMult);
    waitSpin->setLineWidth(3 * sizeMult);
    waitSpin->setInnerRadius(5 * sizeMult);
    waitSpin->setRevolutionsPerSecond(2);
    waitSpin->setColor(QColor(0, 150, 136));

    connect(&spinWatch, &BoolMapOr::valueChanged, [=](bool v){if(v)this->waitSpin->start(); else this->waitSpin->stop();});

    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, [=](){spinWatch.disable("kerTrans");});
    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, &KerPlot::cntSetKernel);
    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::started, this, [=](){spinWatch.enable("kerTrans");});

    connect(&srcKerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, [=](){spinWatch.disable("kerTrans");});
    connect(&srcKerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, &KerPlot::cntSetSrcKernel);
    connect(&srcKerTransWatch, &QFutureWatcher<std::vector<double>>::started, this, [=](){spinWatch.enable("kerTrans");});

    this->axisRect()->setRangeDrag(Qt::Horizontal);
    this->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(this->xAxis, SIGNAL(rangeChanged(const QCPRange&, const QCPRange&)),
            this, SLOT(checkXBounds(const QCPRange&, const QCPRange&)));
    this->setEnabled(false);


    this->addGraph();
    auto pen = QPen(QColor(249, 124, 14));
    pen.setWidth(1);
    this->graph(0)->setPen(pen);
    this->addGraph();    
    pen = QPen(QColor(31, 119, 179));
    pen.setWidth(1);
    this->graph(1)->setPen(pen);
    //this->setNotAntialiasedElements(QCP::aeAll);
//    this->graph(0)->setAdaptiveSampling(false);
//    this->graph(1)->setAdaptiveSampling(false);

    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //default values
    setFreqs(1., 1, 0);
    kerMaxGain = 1.;
    srcKerMaxGain = 1.;
    plotPoints = 30000;
    srcPlotPoints = 10000;

    this->xAxis->setBasePen(QPen(Qt::white, 1));
    this->yAxis->setBasePen(QPen(Qt::white, 1));
    this->xAxis->setTickPen(QPen(Qt::white, 1));
    this->yAxis->setTickPen(QPen(Qt::white, 1));
    this->xAxis->setSubTickPen(QPen(Qt::white, 1));
    this->yAxis->setSubTickPen(QPen(Qt::white, 1));
    this->xAxis->setTickLabelColor(Qt::white);
    this->yAxis->setTickLabelColor(Qt::white);
    this->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    this->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    this->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    this->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    //this->xAxis->grid()->setSubGridVisible(true);
    //this->yAxis->grid()->setSubGridVisible(true);
    this->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    this->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    this->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    this->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    QFont lbFont = this->font();
    if(lbFont.pointSize()>0){
        lbFont.setPointSize(lbFont.pointSize()+2);
        this->xAxis->setLabelFont(lbFont);
        this->yAxis->setLabelFont(lbFont);
    }

    this->xAxis->setLabelColor(Qt::white);
    this->yAxis->setLabelColor(Qt::white);
    this->setBackground(QColor(35, 45, 55));
//    QLinearGradient plotGradient;
//    plotGradient.setStart(0, 0);
//    plotGradient.setFinalStop(0, 350);
//    plotGradient.setColorAt(0, QColor(80, 80, 80));
//    plotGradient.setColorAt(1, QColor(50, 50, 50));
//    QLinearGradient axisRectGradient;
//    axisRectGradient.setStart(0, 0);
//    axisRectGradient.setFinalStop(0, 350);
//    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
//    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
//    customPlot->axisRect()->setBackground(axisRectGradient);


//    QCPTextElement *title = new QCPTextElement(this);
//    title->setText("Plot Title Example");
//    title->setFont(QFont("sans", 12, QFont::Bold));
    // then we add it to the main plot layout:
//    this->plotLayout()->insertRow(0); // insert an empty row above the axis rect
//    this->plotLayout()->addElement(0, 0, title); // place the title in the empty cell we've just created
}

void KerPlot::setKernel(std::shared_ptr<const FirKer> kernel){
    QFuture<std::vector<double> > fut = QtConcurrent::run([=](){return kernel->transmission(plotPoints);});
    kerTransWatch.setFuture(fut);
    plotClearedMeanwhile = false;
}

void KerPlot::cntSetKernel(){
    if(plotClearedMeanwhile) return;
    std::vector<double> trns = kerTransWatch.future().result();
    qDebug() << "trns.size():" <<trns.size();
    //std::vector<double> trns = kernel->transmission(plotPoints);
    if(inverseBand)
        std::reverse(trns.begin(), trns.end());

    transmission = QVector<double>::fromStdVector(trns);
    transmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    if(freqs.size()-1 != plotPoints)
        updateFreqs();

    double max = 1.;
    for(auto v : trns)
        if(v > max) max = v;
    qDebug() << "max gain:" << max;
    if(max > 1000.){
        for(auto v : trns)
            qDebug() << v;
    }
    kerMaxGain = max;

    setPlotType(plotType); //this will replot
}

void KerPlot::setSrcKernel(std::shared_ptr<const FirKer> kernel){
    QFuture<std::vector<double> > fut = QtConcurrent::run([=](){return kernel->transmission(srcPlotPoints);});
    srcKerTransWatch.setFuture(fut);
    srcPlotClearedMeanwhile = false;
}

void KerPlot::cntSetSrcKernel(){

    if(srcPlotClearedMeanwhile) return;
    std::vector<double> trns = srcKerTransWatch.future().result();

    srcTransmission = QVector<double>::fromStdVector(trns);
    srcTransmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    if(srcFreqs.size()-1 != srcPlotPoints)
        updateSrcFreqs();

    double max = 1.;
    qDebug() << "Max trns:";
    for(auto v : trns){
        if(v > max){ max = v;
            qDebug() << v;
        }
    }
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
    srcPlotClearedMeanwhile = true;
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
    this->graph(1)->setData(freqs,transmission,true);
    assert(srcFreqs.size() == srcTransmission.size());
    this->graph(0)->setData(srcFreqs,srcTransmission,true);

    double maxGain = std::max(srcKerMaxGain, kerMaxGain);
    this->yAxis->setRange(QCPRange(0.,(maxGain*1.02)));

    this->setEnabled(true);
    this->replot();
}

void KerPlot::bodePlot(){
    this->yAxis->setLabel(tr("Attenuation, dB"));

    assert(freqs.size() == transmissionBode.size());
    this->graph(1)->setData(freqs,transmissionBode,true);

    assert(srcFreqs.size() == srcTransmissionBode.size());
    this->graph(0)->setData(srcFreqs,srcTransmissionBode,true);

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

void KerPlot::toggleFirTransPlot(bool toggle){
    this->graph(1)->setVisible(toggle);
    replot();
}

void KerPlot::toggleSrcTransPlot(bool toggle){
    this->graph(0)->setVisible(toggle);
    replot();
}

QSize KerPlot::sizeHint() const{
    QSize sh = QCustomPlot::sizeHint();
    return QSize(sh.width()+480, sh.height()+320);
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
