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
    pen = QPen(QColor(132,186,91));
    pen.setWidth(1);
    this->graph(1)->setPen(pen);
    this->addGraph();
    pen = QPen(QColor(114,147,203));
    pen.setWidth(1);
    this->graph(2)->setPen(pen);
    //this->setNotAntialiasedElements(QCP::aeAll);
//    this->graph(0)->setAdaptiveSampling(false);
//    this->graph(1)->setAdaptiveSampling(false);

    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //default values
    setFreqs(1., 1, 0);
    kerMaxGain = 1.;
    srcKerMaxGain = 1.;
    srcPlotDiv = 20000;
    plotDivScale = 1;
    plotDiv = srcPlotDiv*plotDivScale;

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
    QFuture<std::vector<double> > fut = QtConcurrent::run([=](){return kernel->transmission(plotDiv);});
    kerTransWatch.setFuture(fut);
    plotClearedMeanwhile = false;
}

void KerPlot::cntSetKernel(){
    if(plotClearedMeanwhile) return;
    std::vector<double> trns = kerTransWatch.future().result();

    if(inverseBand)
        std::reverse(trns.begin(), trns.end());

    transmission = QVector<double>::fromStdVector(trns);
    transmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    if(freqs.size()-1 != plotDiv)
        updateFreqs();

    double max = 1.;
    for(auto v : trns)
        if(v > max) max = v;
    kerMaxGain = max;

    setPlotType(plotType); //this will replot
}

void KerPlot::setSrcKernel(std::shared_ptr<const FirKer> kernel){
    QFuture<std::vector<double> > fut = QtConcurrent::run([=](){return kernel->transmission((srcPlotDiv*t),(srcPlotDiv*band),(srcPlotDiv*(band+1)));});
    srcKerTransWatch.setFuture(fut);
    srcPlotClearedMeanwhile = false;
}

void KerPlot::cntSetSrcKernel(){

    if(srcPlotClearedMeanwhile) return;
    std::vector<double> trns = srcKerTransWatch.future().result();

    srcTransmission = QVector<double>::fromStdVector(trns);
    srcTransmissionBode = QVector<double>::fromStdVector(FirKer::toBode(trns));

    if(srcFreqs.size()-1 != (srcPlotDiv*t))
        updateSrcFreqs();

    double max = 1.;

    for(const auto& v : trns)
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

void KerPlot::setPlotType(const QString& plotType){
    this->plotType = plotType;
    if(plotType == tr("Bode Plot")){
        bodePlot();
    }
    else if(plotType == tr("Amplitude Plot")){
        amplitudePlot();
    }
}

void KerPlot::amplitudePlot(){
    this->yAxis->setLabel(tr("Amplitude"));

    assert(freqs.size() == transmission.size());
    this->graph(1)->setData(freqs,transmission,true);
    assert(srcFreqs.size() == srcTransmission.size());
    this->graph(0)->setData(srcFreqs,srcTransmission,true);

    if(freqs.size() > 0 && srcFreqs.size() > 0)
        totalAmplTrans();
    else
        clearTotalTrans();

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

    if(freqs.size() > 0 && srcFreqs.size() > 0)
        totalBodeTrans();
    else
        clearTotalTrans();

    double maxGaindB = 20 * std::log10(std::max(srcKerMaxGain, kerMaxGain));
    this->yAxis->setRange(QCPRange(-100., maxGaindB+1));

    this->setEnabled(true);
    this->replot();
}

void KerPlot::totalAmplTrans(){
    QVector<double> trns, freqs;
    trns.resize(srcPlotDiv + 1);
    freqs.resize(srcPlotDiv + 1);
    int beg = srcPlotDiv * band;
    for(int i = 0; i <= srcPlotDiv; ++i){
        trns[i] = transmission[i*plotDivScale] * srcTransmission[beg + i] * srcTransmission[beg + i];
        freqs[i] = srcFreqs[beg + i];
    }
    this->graph(2)->setData(freqs,trns,true);
}


void KerPlot::totalBodeTrans(){
    QVector<double> trns, freqs;
    trns.resize(srcPlotDiv + 1);
    freqs.resize(srcPlotDiv + 1);
    int beg = srcPlotDiv * band;
    for(int i = 0; i <= srcPlotDiv; ++i){
        trns[i] = transmissionBode[i*plotDivScale] + (2*srcTransmissionBode[beg + i]);
        freqs[i] = srcFreqs[beg + i];
    }
    this->graph(2)->setData(freqs,trns,true);
}

void KerPlot::clearTotalTrans(){
    this->graph(2)->setData(QVector<double>(),QVector<double>(),true);
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
    this->t = t; this->band = band;
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
    size_t l = plotDiv + 1;
    this->freqs.resize(l);
    double div = static_cast<double>(l - 1);
    double step = (rBandLimit - lBandLimit)/div;
    for(int i = 0; i < l; ++i){
        this->freqs[i] = lBandLimit + static_cast<double>(i) * step;
    }
}

void KerPlot::updateSrcFreqs(){
    size_t l = (srcPlotDiv*t) + 1;
    this->srcFreqs.resize(l);
    double div = static_cast<double>(l - 1);
    double step = nqFreq/div;
    for(int i = 0; i < l; ++i){
        this->srcFreqs[i] = static_cast<double>(i) * step;
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

void KerPlot::toggleTotalTransPlot(bool toggle){
    this->graph(2)->setVisible(toggle);
    replot();
}

QSize KerPlot::sizeHint() const{
    QSize sh = QCustomPlot::sizeHint();
    return QSize(sh.width()+480, sh.height()+320);
}
