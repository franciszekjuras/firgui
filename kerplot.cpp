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
    QCustomPlot(parent),
    lastMoveEvent(QEvent::MouseMove, QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier)
{

    waitSpin = new WaitingSpinnerWidget(this, true, true);

    int sizeMult = 2;
    waitSpin->setRoundness(70.0);
    waitSpin->setMinimumTrailOpacity(50.0);
    waitSpin->setTrailFadePercentage(70.0);
    waitSpin->setNumberOfLines(24);
    waitSpin->setLineLength(6 * sizeMult);
    waitSpin->setLineWidth(3 * sizeMult);
    waitSpin->setInnerRadius(5 * sizeMult);
    waitSpin->setRevolutionsPerSecond(2);
    waitSpin->setColor(QApplication::palette().highlight().color());

    perfModeTimer.setSingleShot(true);
    perfModeTimeoutMS = 400;
    connect(&perfModeTimer, &QTimer::timeout, this, &KerPlot::exitPerfMode);

    connect(&spinWatch, &BoolMapOr::valueChanged, [=](bool v){if(v)this->waitSpin->start(); else this->waitSpin->stop();});

    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, [=](){spinWatch.disable("kerTrans");});
    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, &KerPlot::cntSetKernel);
    connect(&kerTransWatch, &QFutureWatcher<std::vector<double>>::started, this, [=](){spinWatch.enable("kerTrans");});

    connect(&srcKerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, [=](){spinWatch.disable("srcKerTrans");});
    connect(&srcKerTransWatch, &QFutureWatcher<std::vector<double>>::finished, this, &KerPlot::cntSetSrcKernel);
    connect(&srcKerTransWatch, &QFutureWatcher<std::vector<double>>::started, this, [=](){spinWatch.enable("srcKerTrans");});

    this->axisRect()->setRangeDrag(Qt::Horizontal);
    this->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(this->xAxis, static_cast<void (QCPAxis::*)(const QCPRange&)>(&QCPAxis::rangeChanged), this, &KerPlot::checkXBounds);
    this->setEnabled(false);

    qDebug() << "Axis rect:" << axisRect()->rect();

    penWidth = 2.;
    highlightWidth = 4.;
    this->addGraph();
    auto pen = QPen(QColor(132,186,91));
    pen.setWidthF(penWidth);
    this->graph(0)->setPen(pen);

    this->addGraph();    
    pen = QPen(QColor(255, 119, 0));
    pen.setWidthF(penWidth);
    this->graph(1)->setPen(pen);

    this->addGraph();
    pen = QPen(QColor(114,147,203));
    pen.setWidthF(penWidth);
    this->graph(2)->setPen(pen);

    this->addGraph();
    pen = QPen(QColor(211,1,2));
    pen.setWidthF(penWidth);
    this->graph(3)->setPen(pen);

    this->addGraph();
    pen = QPen(QColor::fromHsv(0,192,255));
    pen.setWidthF(highlightWidth);
    this->graph(4)->setPen(pen);

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

    this->xAxis->setLabelColor(Qt::white);
    this->yAxis->setLabelColor(Qt::white);
    this->setBackground(QColor(0, 43, 54));

    specFocusType = SpecFocus::none;
    specFIdx = 0;
    specGIdx = 0;
    specMouseCapt = false;
}

void KerPlot::mouseMoveEvent(QMouseEvent *event){
    lastMoveEvent = *event;
    mouseMoveEventHandler(event);
}

void KerPlot::mouseMoveEventHandler(QMouseEvent *event, bool insideEvent){
    lastMoveEvent = *event;
    curPos = event->pos();
//    qDebug()<< "move:" << event->pos().x() << event->pos().y();
//    qDebug() << "pos:" << xAxis->pixelToCoord(curPos.x()) << yAxis->pixelToCoord(curPos.y());

    if(!isSpecEditEn()){
        if(!insideEvent) QCustomPlot::mouseMoveEvent(event);
        return;
    }

    if(specMouseCapt){
        //handle specification edits

    } else{//(!specMouseCapt)
        //check cursor position
        if(specFocusType == SpecFocus::none){
            //check if something was focused
            if(checkFocusGrab()){
                //action if something was focused
                handleFocusGrab();
            } else{ //(!checkFocusGrab())
                //do nothing
            }
        } else{ //(specFocusType != SpecFocus::none)
            //check if object is still focused
            if(checkFocusLost()){
                //action if focus was lost
                if(checkFocusGrab()){
                    handleFocusGrab();
                } else{
                    handleFocusLost();
                }
            } else{//(!checkFocusLost())
                //do nothing
            }
        }
        if(!insideEvent) QCustomPlot::mouseMoveEvent(event);
    }
}

bool KerPlot::checkFocusGrab(){
    if(curPos.x() < xCTP(specFreqs[specFIdx])){
        while(specFIdx > 0 &&
              (xCTP(specFreqs[specFIdx]) + xCTP(specFreqs[specFIdx-1]) > 2*curPos.x()))
            --specFIdx;
    } else{
        while(specFIdx < (specFreqs.size()-1) &&
              (xCTP(specFreqs[specFIdx]) + xCTP(specFreqs[specFIdx+1]) < 2*curPos.x()))
            ++specFIdx;
    }
    specGIdx = specFIdx;
    if(curPos.x() > xCTP(specFreqs[specFIdx]))
        ++specGIdx;

    double glP = yCTP(specGains[specFIdx]);
    double grP = yCTP(specGains[specFIdx+1]);
    if( (qAbs(curPos.x()-xCTP(specFreqs[specFIdx])) < KERPLOT_FOCUS_DIST) &&
            (curPos.y() > qMin(glP, grP) - KERPLOT_FOCUS_DIST) &&
            (curPos.y() < qMax(glP, grP) + KERPLOT_FOCUS_DIST) &&
            axisRect()->rect().contains(curPos) ){
        specFocusType = SpecFocus::band;
        return true;
    }

    if( (qAbs(curPos.y()-yCTP(specGains[specGIdx])) < KERPLOT_FOCUS_DIST) &&
            axisRect()->rect().contains(curPos) ){
        specFocusType = SpecFocus::gain;
        return true;
    }

    return false;
}

bool KerPlot::checkFocusLost(){
    if(specFocusType == SpecFocus::band){
        double glP = yCTP(specGains[specFIdx]);
        double grP = yCTP(specGains[specFIdx+1]);
        if( (qAbs(curPos.x()-xCTP(specFreqs[specFIdx])) < KERPLOT_FOCUS_DIST) &&
                (curPos.y() > qMin(glP, grP) - KERPLOT_FOCUS_DIST) &&
                (curPos.y() < qMax(glP, grP) + KERPLOT_FOCUS_DIST) &&
                axisRect()->rect().contains(curPos) ){
            return false;
        }
    } else{ //specFocusType == SpecFocus::gain
        if( ( qAbs(curPos.y()-yCTP(specGains[specGIdx])) < KERPLOT_FOCUS_DIST ) &&
                (curPos.x() > xCTP(specTransFreqs[specGIdx*2]) - KERPLOT_FOCUS_DIST) &&
                (curPos.x() < xCTP(specTransFreqs[specGIdx*2+1]) + KERPLOT_FOCUS_DIST) &&
                axisRect()->rect().contains(curPos)){
            return false;
        }
    }
    specFocusType = SpecFocus::none;
    return true;
}

double KerPlot::xCTP(double v){return xAxis->coordToPixel(v);}
double KerPlot::yCTP(double v){return yAxis->coordToPixel(v);}
double KerPlot::xPTC(double v){return xAxis->pixelToCoord(v);}
double KerPlot::yPTC(double v){return xAxis->pixelToCoord(v);}


void KerPlot::handleFocusGrab(){
    highlightFocus();
    if(specFocusType==SpecFocus::band){
        qDebug() << "Band number" << specFIdx << "was focused";
    } else{
        qDebug() << "Gain number" << specGIdx << "was focused";
    }
}

void KerPlot::handleFocusLost(){
    removeFocusHighlight();
    qDebug() << "Focus lost";
}

void KerPlot::highlightFocus(){
    highlightFreqs.clear();
    highlightGains.clear();

    int startIdx;
    if(specFocusType == SpecFocus::band){
        startIdx = specFIdx*2 + 1;
    } else{ //specFocusType == SpecFocus::gain
        startIdx = specGIdx*2;
    }

    highlightFreqs.push_back(specTransFreqs[startIdx]);
    highlightFreqs.push_back(specTransFreqs[startIdx+1]);
    highlightGains.push_back(specTransGains[startIdx]);
    highlightGains.push_back(specTransGains[startIdx+1]);

    setPlotType(plotType);
}

void KerPlot::removeFocusHighlight(){
    highlightFreqs.clear();
    highlightGains.clear();

    setPlotType(plotType);
}

void KerPlot::mousePressEvent(QMouseEvent *event){
    qDebug()<< "press:" << event->pos().x() << event->pos().y();

    QCustomPlot::mousePressEvent(event);
}

void KerPlot::mouseReleaseEvent(QMouseEvent *event){
    qDebug()<< "release:" << event->pos().x() << event->pos().y();

    QCustomPlot::mouseReleaseEvent(event);
}

void KerPlot::mouseDoubleClickEvent(QMouseEvent *event){
    qDebug()<< "dblclick:" << event->pos().x() << event->pos().y();

    QCustomPlot::mouseDoubleClickEvent(event);
}

void KerPlot::wheelEvent(QWheelEvent* event){
    mouseMoveEventHandler(&lastMoveEvent, true);
    QCustomPlot::wheelEvent(event);
}

void KerPlot::leaveEvent(QEvent* event){
    if(isSpecEditEn() && !specMouseCapt &&
            specFocusType!=SpecFocus::none){
        specFocusType = SpecFocus::none;
        handleFocusLost();
    }

    QCustomPlot::leaveEvent(event);
}



void KerPlot::setSpec(QVector<double> freqs, QVector<double> gains){
    specFreqs = freqs;
    specGains = gains;
    calcSpecTrans();

    setPlotType(plotType);
}

void KerPlot::calcSpecTrans(){
    specTransFreqs.clear(); specTransGains.clear();

    specTransFreqs.push_back(lBandLimit);
    for(auto freq : specFreqs){
        specTransFreqs.push_back(freq);
        specTransFreqs.push_back(freq);
    }
    specTransFreqs.push_back(rBandLimit);

    for(auto gain : specGains){
        specTransGains.push_back(gain);
        specTransGains.push_back(gain);
    }
}

void KerPlot::setKernel(std::shared_ptr<const FirKer> kernel, double roiL, double roiR){
    QFuture<std::vector<double> > fut = QtConcurrent::run([=](){return kernel->transmission(plotDiv);});
    kerTransWatch.setFuture(fut);
    plotClearedMeanwhile = false;
    if(roiL < roiR)
        this->xAxis->setRange(roiL/unitMult, roiR/unitMult);
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

void KerPlot::clearSpec(){
    specFreqs.clear();
    specGains.clear();
    specTransFreqs.clear();
    specTransGains.clear();
    specFIdx = 0;
    specGIdx = 0;
    specFocusType = SpecFocus::none;
    specMouseCapt = false;
    setPlotType(plotType);
}

void KerPlot::resetPlot(double freq, int t, int band){
    setFreqs(freq, t, band);
    clearKernel();
    clearSrcKernel();
    clearSpec();
    removeFocusHighlight();
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

void KerPlot::enterPerfMode(){
    bool wasActive = perfModeTimer.isActive();
    perfModeTimer.start(perfModeTimeoutMS);
    if(wasActive)
        return;
    for(int i = 0; i < 5; ++i){
        auto pen = graph(i)->pen();
        pen.setWidthF(1.);
        graph(i)->setPen(pen);
    }
}

void KerPlot::exitPerfMode(){
    for(int i = 0; i < 4; ++i){
        auto pen = graph(i)->pen();
        pen.setWidthF(penWidth);
        graph(i)->setPen(pen);
    }
    auto pen = graph(4)->pen();
    pen.setWidthF(highlightWidth);
    graph(4)->setPen(pen);
    replot();
}

//void KerPlot::setPlotPerf(const QString& plotPerf){
//    if(plotPerf == tr("High performance")){
//        for(int i = 0; i < 3; ++i){
//            auto pen = graph(i)->pen();
//            pen.setWidth(1);
//            graph(i)->setPen(pen);
//        }
//        replot();
//    }
//    else if(plotPerf == tr("High quality")){
//        for(int i = 0; i < 3; ++i){
//            auto pen = graph(i)->pen();
//            pen.setWidth(3);
//            graph(i)->setPen(pen);
//        }
//        replot();
//    }
//}

void KerPlot::amplitudePlot(){
    this->yAxis->setLabel(tr("Amplitude"));
    this->yAxis->setTickLabelColor(QColor(0,0,0));
    this->yAxis->setTickLabelColor(QColor(255,255,255));

    assert(freqs.size() == transmission.size());
    this->graph(1)->setData(freqs,transmission,true);
    assert(srcFreqs.size() == srcTransmission.size());
    this->graph(0)->setData(srcFreqs,srcTransmission,true);

    if(freqs.size() > 0 && srcFreqs.size() > 0)
        totalAmplTrans();
    else
        clearTotalTrans();

    assert(specTransFreqs.size() == specTransGains.size());
    this->graph(3)->setData(specTransFreqs,specTransGains,true);

    assert(highlightFreqs.size() == highlightGains.size());
    this->graph(4)->setData(highlightFreqs,highlightGains,true);

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

    this->graph(3)->setData(QVector<double>(),QVector<double>(),true);

    this->graph(4)->setData(QVector<double>(),QVector<double>(),true);

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

void KerPlot::checkXBounds(const QCPRange& newRange){
    if(newRange.lower < this->xRange.lower || newRange.upper > this->xRange.upper){
        if(newRange.size() > this->xRange.size()){
            this->xAxis->setRange(this->xRange);
        }
        else if(newRange.lower < this->xRange.lower){
            this->xAxis->setRange(xRange.lower, xRange.lower + newRange.size());
        }
        else{
            this->xAxis->setRange(xRange.upper - newRange.size(), xRange.upper);
        }
    }
    enterPerfMode();
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
        unitMult = 1000.;
        this->xAxis->setLabel(tr("Frequency, MHz"));
    }else{
        unitMult = 1.;
        this->xAxis->setLabel(tr("Frequency, kHz"));        
    }

    this->xRange = QCPRange(lBandLimit, rBandLimit);
    this->xAxis->setRange(xRange);

}

void KerPlot::updateFreqs(){
    int l = plotDiv + 1;
    this->freqs.resize(l);
    double div = static_cast<double>(l - 1);
    double step = (rBandLimit - lBandLimit)/div;
    for(int i = 0; i < l; ++i){
        this->freqs[i] = lBandLimit + static_cast<double>(i) * step;
    }
}

void KerPlot::updateSrcFreqs(){
    int l = (srcPlotDiv*t) + 1;
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

void KerPlot::toggleSpecTransPlot(bool toggle){
    this->graph(3)->setVisible(toggle);
    replot();
}

bool KerPlot::isSpecEditEn(){
    return (this->graph(3)->visible() && !this->graph(3)->data()->isEmpty());
}

QSize KerPlot::sizeHint() const{
    QSize sh = QCustomPlot::sizeHint();
    return QSize(sh.width()+480, sh.height()+320);
}
