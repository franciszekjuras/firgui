#include <QtWidgets>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <memory>
#include "groupspecs.h"
#include "firker.h"
#include "waitingspinnerwidget.h"
#ifdef _WIN32
#include "delegate.h"
#endif
#include "clicklabel.h"
#include "xcolor.h"

#define PASSBAND_WIDTH_RATIO 4.11 // magic numbers for .1% rippling in passband
                                  // and -80 dB (.01%) attenuation in stopband
#ifdef _WIN32
GroupSpecs::GroupSpecs(QWidget *parent) :QWidget(parent)
{
//part:layout

QVBoxLayout* groupVBox = new QVBoxLayout;
this->setLayout(groupVBox);

QLabel* titleLabel = new QLabel(/*QString("<b>") + */tr("Filter specification") /*+ "</b>"*/);
titleLabel->setContentsMargins(5,0,0,5);
titleLabel->setFont(QFont("Roboto Medium",14));
groupVBox->addWidget(titleLabel);
QPalette pal;
pal.setColor(titleLabel->foregroundRole(),pal.color(QPalette::Inactive, QPalette::WindowText));
titleLabel->setPalette(pal);
connect(this, &GroupSpecs::updatePalette, [=](){QPalette pal;
    pal.setColor(titleLabel->foregroundRole(),pal.color(QPalette::Inactive, QPalette::WindowText));
    titleLabel->setPalette(pal);});
qApp->installEventFilter(this);
//neccessary to keep up with palette changes

QWidget* groupContent = new QWidget;
groupVBox->addWidget(groupContent);

//setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
QVBoxLayout* mainVBox = new QVBoxLayout;
mainVBox->setContentsMargins(0,0,0,0);
groupContent->setLayout(mainVBox);
#else
GroupSpecs::GroupSpecs(QWidget *parent) :QGroupBox(tr("Filter specification"),parent)
{
//part:layout

setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
QVBoxLayout* mainVBox = new QVBoxLayout;
this->setLayout(mainVBox);
#endif
//mainVBox --v

    QGridLayout* specificationGrid = new QGridLayout;
    mainVBox->addLayout(specificationGrid);
    //specificationGrid --v

        freqsLineEdit = new QLineEdit;
        specificationGrid->addWidget(new QLabel(tr("Frequencies")),0,0);
        specificationGrid->addWidget(freqsLineEdit, 0, 1);
        freqsLineEdit->setToolTip(tr("Space separated corner frequencies (in band range)."));

        QComboBox* unitCombo = new QComboBox;
#ifdef _WIN32
        unitCombo->view()->setItemDelegate(new PopupItemDelegate(unitCombo));
#endif
        specificationGrid->addWidget(unitCombo, 0, 2);
        unitCombo->setFocusPolicy(Qt::ClickFocus);
        unitCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        gainsLineEdit = new QLineEdit;
        specificationGrid->addWidget(new QLabel(tr("Gains")),1,0);
        specificationGrid->addWidget(gainsLineEdit, 1, 1);
        gainsLineEdit->setToolTip(tr("Space separated gains (1 for passband, 0 for stopband)."));

        QComboBox* windowCombo = new QComboBox;
#ifdef _WIN32
        windowCombo->view()->setItemDelegate(new PopupItemDelegate(windowCombo));
#endif
        specificationGrid->addWidget(new QLabel(tr("Window")),2,0);
        specificationGrid->addWidget(windowCombo, 2, 1,1,2);
        windowCombo->setToolTip("Different windows give various roll-off/rippling tradeoffs.");
        windowCombo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        windowCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        bandCombo = new QComboBox;
#ifdef _WIN32
        bandCombo->view()->setItemDelegate(new PopupItemDelegate(bandCombo));
#endif
        specificationGrid->addWidget(new QLabel(tr("Working Band")),3,0);
        specificationGrid->addWidget(bandCombo, 3, 1, 1, 2);
        bandCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        bandCombo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);


    QHBoxLayout* buttonsHBox = new QHBoxLayout;
    mainVBox->addLayout(buttonsHBox);
    //buttonsHBox --v

        ClickLabel* helpClickLabel = new ClickLabel("<big>?</big>");
        buttonsHBox->addWidget(helpClickLabel);

        buttonsHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Fixed));

        waitSpin = new WaitingSpinnerWidget(nullptr, false, false);
        buttonsHBox->addWidget(waitSpin);
        waitSpin->setRoundness(70.0);   waitSpin->setMinimumTrailOpacity(50.0); waitSpin->setTrailFadePercentage(70.0);
        waitSpin->setNumberOfLines(24); waitSpin->setLineLength(6);             waitSpin->setLineWidth(3);
        waitSpin->setInnerRadius(5);    waitSpin->setRevolutionsPerSecond(2);
        waitSpin->setColor(QApplication::palette().highlight().color());

        QPushButton* calculateButton = new QPushButton(tr("Calculate"));
        buttonsHBox->addWidget(calculateButton);

        QPushButton* setButton = new QPushButton(tr("Set"));
        buttonsHBox->addWidget(setButton);

//part:function

    //:this
    connect(this, &GroupSpecs::enableCalculateButton, calculateButton,  &QPushButton::setEnabled);
    connect(this, &GroupSpecs::enableSetButton, setButton,  &QPushButton::setEnabled);
//    connect(this, &GroupSpecs::srcKernelClear, [this](){srcKernelReady(false);});
//    connect(this, &GroupSpecs::kernelClear, [this](){kernelReady(false);});
    connect(this, &GroupSpecs::srcKernelChanged, [this](){srcKernelReady(true);});
    connect(this, &GroupSpecs::kernelChanged, [this](){kernelReady(true);});
    connect(this, &GroupSpecs::resetPlot, [this](){srcKernelReady(false);kernelReady(false);});

    //:freqsLineEdit|:gainsLineEdit
    connect(freqsLineEdit, &QLineEdit::returnPressed, gainsLineEdit, static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
    connect(gainsLineEdit, &QLineEdit::returnPressed, calculateButton, &QPushButton::click);
    connect(freqsLineEdit, &QLineEdit::textChanged, [=](QString text){ QString oldText = text; text.replace(',','.');
        if(text!=oldText){
            int cp = freqsLineEdit->cursorPosition();
            freqsLineEdit->setText(text);
            freqsLineEdit->setCursorPosition(cp);
        }
    });
    connect(gainsLineEdit, &QLineEdit::textChanged, [=](QString text){ QString oldText = text; text.replace(',','.');
        if(text!=oldText){
            int cp = gainsLineEdit->cursorPosition();
            gainsLineEdit->setText(text);
            gainsLineEdit->setCursorPosition(cp);
        }
    });
    //:unitCombo
    connect(unitCombo, &QComboBox::currentTextChanged, this, &GroupSpecs::unitChanged);
    unitCombo->addItem("kHz"); unitCombo->addItem("MHz");

    //:windowCombo
    connect(windowCombo, &QComboBox::currentTextChanged, this, &GroupSpecs::windowChanged);
    QStringList windows;
    windows << tr("None") << tr("Hamming") << tr("Blackman");
    windowCombo->addItems(windows);
    windowCombo->setCurrentText(tr("Hamming"));

    //:bandCombo
    connect(bandCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GroupSpecs::bandChanged);

    //:helpClickLabel|:calculateButton|:setButton
    connect(helpClickLabel, &ClickLabel::clicked, this, &GroupSpecs::showHelp);
    connect(calculateButton, &QPushButton::clicked, this, &GroupSpecs::calculateKernel);
    connect(setButton, &QPushButton::clicked, this, &GroupSpecs::setKernels);
    enableCalculateButton(false); enableSetButton(false);

    //:waitSpin
    connect(&spinWatch, &BoolMapOr::valueChanged, [=](bool v){if(v)this->waitSpin->start(); else this->waitSpin->stop();});

    //:calculateKernelWatch
    connect(&calculateKernelWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::started, this, [=](){spinWatch.enable("calculateKernel");});
    connect(&calculateKernelWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::finished, this, &GroupSpecs::calculateKernelFinished);

    //:calculateSrcKernelWatch
    connect(&calculateSrcKernelWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::started, this, [=](){spinWatch.enable("calculateSrcKernel");});
    connect(&calculateSrcKernelWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::finished, this, &GroupSpecs::calculateSrcKernelFinished);


    isKernelReady = false;          isSrcKernelReady = false;
    isFilterReady = false;          isSrcKernelLoaded = false;
    pendingCalculateKernel = false; pendingCalculateSrcKernel = false;

}

void GroupSpecs::handleConnect(bool is){
    if(!is){
        filterReady(false);
        isSrcKernelLoaded = false;
    }
}

void GroupSpecs::windowChanged(QString windowStr){
    if(windowStr == tr("None"))
        currentWindow = LeastSqFirKer::Window::none;
    else if(windowStr == tr("Hamming"))
        currentWindow = LeastSqFirKer::Window::hamming;
    else if(windowStr == tr("Blackman"))
        currentWindow = LeastSqFirKer::Window::blackman;
    else
        assert(windowStr.isEmpty());
}

void GroupSpecs::unitChanged(QString unit){
    double newUnitMultiplier;
    if(unit == "kHz")
        newUnitMultiplier = 1.;
    else if(unit == "MHz")
        newUnitMultiplier = 1000.;
    else
        assert(false);

    std::vector<double> freqs;

    if(newUnitMultiplier != unitMultiplier && textToDoubles(freqsLineEdit->text().toStdString(),freqs)){
        double ratio = unitMultiplier / newUnitMultiplier;
        QString newFreqs;
        for(auto& v : freqs){
            v *= ratio;
            newFreqs += QString::number(v) += " ";
        }
        newFreqs.chop(1);
        freqsLineEdit->setText(newFreqs);
    }
    unitMultiplier = newUnitMultiplier;
}

void GroupSpecs::calculateKernel(){
    kernelReady(false);
    if(calculateKernelWatch.isRunning()){pendingCalculateKernel = true; return;}
    pendingCalculateKernel = false;

    LeastSqFirKer ker;
    double kerSamplingFreq = fpgaSamplingFreq / static_cast<double>(t);
    ker.setSamplingFreq(kerSamplingFreq);
    ker.setRank(t * d);
    std::vector<double> freqs, gains;

    if(!textToDoubles(freqsLineEdit->text().toStdString(),freqs) ||
        !textToDoubles(gainsLineEdit->text().toStdString(),gains)){
        qWarning() << "Incorrect filter specification.\nFreqs:" << freqsLineEdit->text() << "\nGains:" << gainsLineEdit->text();
        int uc = QMessageBox::warning(this, tr(WINDOW_TITLE),tr("Could not parse filter specification. Make sure that all inserted values are proper numbers and are separated by spaces only."),tr("Help"),tr("Close"),QString(),1, 1);
        if(uc == 0)showHelp();
        return;
    }

    //unit conversion
    for(auto& v : freqs){
        v *= unitMultiplier;
    }

    if(!isSpecValid(freqs, gains)){
        qWarning() << "Incorrect filter specification.\nFreqs:" << freqsLineEdit->text() << "\nGains:" << gainsLineEdit->text();
        int uc = QMessageBox::warning(this, tr(WINDOW_TITLE),tr("Incorrect filter specification. Make sure that number of gains is bigger by one from number of frequencies, that frequencies are ordered and in limits of working band, and that gains are non-negative."),tr("Help"),tr("Close"),QString(),1, 1);
        if(uc == 0)showHelp();
        return;
    } else{
        emit textSpecChanged(QVector<double>::fromStdVector(freqs),QVector<double>::fromStdVector(gains));
    }

    //ROI calculation
    if(freqs.size() > 0){
        double roiConst = 800000.; int roiDiv = t*t*d;
        double roiExt = qMax((roiConst/roiDiv), (freqs.back() - freqs.front())*0.3);
            roiL = freqs.front() - roiExt;
            roiR = freqs.back() + roiExt;
    } else{
        roiL = 0.; roiR = 0.;
    }
    //end

    int band = currentBand();
    double kerNyquistFreq = kerSamplingFreq / 2.;

    for(auto& v : freqs)
        v -= (kerNyquistFreq*band); //shift

    if((band%2) == 1){
        //band reversal
        for(auto& v : freqs)
            v = kerNyquistFreq - v; //reversal

        std::reverse(freqs.begin(),freqs.end());
        std::reverse(gains.begin(),gains.end());
    }

    if(!ker.setSpecification(freqs, gains)){
        qWarning() << "Incorrect filter specification.\nFreqs:" << freqsLineEdit->text() << "\nGains:" << gainsLineEdit->text();
        int uc = QMessageBox::warning(this, tr(WINDOW_TITLE),tr("Incorrect filter specification. Make sure that number of gains is bigger by one from number of frequencies, that frequencies are ordered and in limits of working band, and that gains are non-negative."),tr("Help"),tr("Close"),QString(),1, 1);
        if(uc == 0)showHelp();
        return;
    }
    ker.setWindow(currentWindow);

    std::shared_ptr<FirKer> ker_shared = std::make_shared<LeastSqFirKer>(ker);
    qInfo() << "Starting kernel calculation.\nFreqs:" << freqsLineEdit->text() << "\nGains:" << gainsLineEdit->text();
    QFuture<std::shared_ptr<FirKer>> fut = QtConcurrent::run([=](){ker_shared->calc(); return ker_shared;});
    calculateKernelWatch.setFuture(fut);
}

void GroupSpecs::calculateKernelFinished(){
    spinWatch.disable("calculateKernel");
    if(pendingCalculateKernel){calculateKernel();return;}

    std::shared_ptr<FirKer> ker = calculateKernelWatch.future().result();

    if(!ker->isValid()){
        qWarning() << "Kernel calculation failed because of incorrect specification.";
        int uc = QMessageBox::warning(this, tr(WINDOW_TITLE),tr("Incorrect filter specification. Make sure that number of gains is bigger by one from number of frequencies, that frequencies are ordered and in limits of working band, and that gains are non-negative."),tr("Help"),tr("Close"),QString(),1, 1);
        if(uc == 0)showHelp();
        return;
    }

    qInfo() << "Filter kernel calculated.";
    crrKer = ker->getKernel();
    emit kernelChanged(ker, roiL, roiR);
}

void GroupSpecs::calculateSrcKernel(){
    kernelReady(false); isSrcKernelLoaded = false;

    if(calculateSrcKernelWatch.isRunning()){
        pendingCalculateSrcKernel = true; return;
    }
    pendingCalculateSrcKernel = false;
    // normalized frequency specification -- to avoid double conversion
    if(t == 1) return;

    std::vector<double> freqs, gains, weights;
    int band = currentBand();
    double dband = static_cast<double>(band);
    double dT = static_cast<double>(t);
    int srcKerRank = t * s;
    double width = PASSBAND_WIDTH_RATIO / static_cast<double>(srcKerRank);
    double stopBandWeight = 10.;

    if((band!=0) && (band != (t-1)) && (width > .5/dT/2.)){
        qCritical() << "Higher bands are not supported in this configuration. Band:" << band << " t:" << t;
        QMessageBox::critical(this, tr(WINDOW_TITLE), tr("Unexpected error: cannot calculate SRC kernel for this band."));
        return;
    }

    if(band > 0){
        gains.push_back(0); // 0 frequency
        freqs.push_back(.5/dT*(dband + .0001)); gains.push_back(0); // small shift added to prevent kernel calculation issues
        freqs.push_back(.5/dT*dband + width); // max frequency
        weights.push_back(stopBandWeight);
    }
    else if(width > .5/dT) {
        qCritical() << "SRC kernel rank too low.";
        QMessageBox::critical(this, tr(WINDOW_TITLE), tr("This configuration is not supported with this SRC kernel specification."));
        return;
    }
    gains.push_back(1); gains.push_back(1);
    weights.push_back(1.);

    if((band + 1) < t){
        freqs.push_back(.5/dT*(dband+1.) - width); freqs.push_back(.5/dT*(dband+1.-.0001));
        gains.push_back(0); gains.push_back(0);
        weights.push_back(stopBandWeight);
    }

    EqRippleFirKer ker;
    ker.setSamplingFreq(1.);
    ker.setRank(srcKerRank);

    if(!ker.setSpecification(freqs,gains,weights)){
        qCritical() << "Wrong equiripple filter specification.";
        QMessageBox::critical(this, tr(WINDOW_TITLE), tr("Unexpected error: wrong equiripple filter specification."));
        return;
    }

    std::shared_ptr<FirKer> ker_shared = std::make_shared<EqRippleFirKer>(ker);
    qInfo() << "Starting SRC kernel calculation.";
    QFuture<std::shared_ptr<FirKer>> fut = QtConcurrent::run([=](){ker_shared->calc(); return ker_shared;});
    calculateSrcKernelWatch.setFuture(fut);
}

void GroupSpecs::calculateSrcKernelFinished(){
    spinWatch.disable("calculateSrcKernel");
    if(pendingCalculateSrcKernel){calculateSrcKernel();return;}


    std::shared_ptr<FirKer> ker = calculateSrcKernelWatch.future().result();
    if(!ker->isValid()){
        qCritical() << "Src kernel calculation failed.";
        QMessageBox::critical(this, tr(WINDOW_TITLE), tr("Unexpected error: SRC kernel calculation failed."));
        return;
    }

    qInfo() << "SRC kernel calculated.";
    crrSrcKer = ker->getKernel();
    //this covers situation when firpm changes kernel size
    while(crrSrcKer.size() < ker->getRank()){
        qInfo() << "Adding 0 to src kernel.";
        crrSrcKer.push_back(0.);
    }

    ker->setSamplingFreq(fpgaSamplingFreq);
    emit srcKernelChanged(ker);
}

void GroupSpecs::setKernels(){
    qInfo() << "Set clicked.";
    if(!isSrcKernelLoaded)
        reqLoadSrcKernel(crrSrcKer);
    else
        qInfo() << "SRC kernel already loaded.";
    isSrcKernelLoaded = true;
    reqLoadKernel(crrKer);
}

void GroupSpecs::setSpec(QVector<double> freqs, QVector<double> gains){
    QString newFreqs;
    for(auto& v : freqs){
        v /= unitMultiplier;
        newFreqs += QString::number(v) += " ";
    }
    newFreqs.chop(1);

    QString newGains;
    for(auto& v : gains){
        newGains += QString::number(v) += " ";
    }
    newGains.chop(1);

    freqsLineEdit->setText(newFreqs);
    gainsLineEdit->setText(newGains);
}

void GroupSpecs::bitstreamChanged(QMap<QString, int> specs){
    filterReady(false);

    qInfo() << "Bitstream changed:" << specs;

    bool validT = (specs.contains("tm") && specs["tm"] > 0);
    bool validD = (specs.contains("fb") && specs["fb"] > 0);
    bool validS = (specs.contains("sb") && specs["sb"] > 0);

    int tOld = t; t = validT ? specs["tm"] : 0;
    int dOld = d; d = validD ? specs["fb"] : 0;
    int sOld = s; s = validS ? specs["sb"] : 0;

    if(!validT){
       bandCombo->clear(); enableCalculateButton(false); resetPlot(fpgaSamplingFreq,1,0);return;
    }
    if(t == tOld && validS && s == sOld && validD && d == dOld)
        qInfo() << "No changes in bitstream.";
    else
        rebuild();

    enableCalculateButton(validD);
}

void GroupSpecs::rebuild(){
    qInfo() << "Rebuilding band combo.";
    bandCombo->clear();
    double bandW = fpgaSamplingFreq / t / 2.;
    QString unit(" kHz");

    int srcKerRank = t * s;
    double width = PASSBAND_WIDTH_RATIO / static_cast<double>(srcKerRank);
    middleBandsEn = (width < .5/static_cast<double>(t)/2.);

    for(int i = 0.; i < t; ++i){
        double d_i = static_cast<double>(i);
        if((d_i + 1.)*bandW >= 1000.){
            bandW /= 1000.;
            unit = " MHz";
        }
        if(middleBandsEn || i == 0 || i == (t -1))
        bandCombo->addItem(QString::number(d_i * bandW,'g',4) + " - " + QString::number((d_i+1) * bandW,'g',4) + unit);
    }
}

void GroupSpecs::bandChanged(int band){
    if(band < 0) return;
    qInfo() << "Band changed to " << band;
    resetPlot(fpgaSamplingFreq, t, currentBand());
    freqsLineEdit->setText("");
    gainsLineEdit->setText("1");
    textSpecChanged({},{1.});
    calculateSrcKernel();
}

void GroupSpecs::bitstreamLoaded(QMap<QString, int> specs){
    qInfo() << "Bitstream loaded:" << specs;
    bitstreamChanged(specs);
    filterReady(true);
}

void GroupSpecs::filterReady(bool en){
    isFilterReady = en;
    enableSetButton(isFilterReady && isKernelReady && isSrcKernelReady);
}

void GroupSpecs::kernelReady(bool en){
    isKernelReady = en;
    enableSetButton(isFilterReady && isKernelReady && isSrcKernelReady);
}

void GroupSpecs::srcKernelReady(bool en){
    isSrcKernelReady = en;
    enableSetButton(isFilterReady && isKernelReady && isSrcKernelReady);
}

void GroupSpecs::setfpgaSamplingFreq(double freq){
    fpgaSamplingFreq = freq;
}

bool GroupSpecs::textToDoubles(const std::string& str, std::vector<double>& v){
    v.clear();
    std::stringstream ss(str, std::ios_base::in);
    double tmpV;
    while(ss>>tmpV)
        v.push_back(tmpV);
    if(!ss.eof()){
        v.clear();
        return false;
    }
    return true;
}

int GroupSpecs::currentBand(){
    int band = bandCombo->currentIndex(); assert (band >= 0); assert (band < t);
    if(!middleBandsEn && (band == 1)) band = t - 1;
    return band;
}

bool GroupSpecs::isSpecValid(const std::vector<double> &freqs, const std::vector<double> &gains){
    double kerNyqstFreq = fpgaSamplingFreq / static_cast<double>(2*t);
    int band = currentBand();
    double llim = static_cast<double>(band) * kerNyqstFreq;
    double rlim = llim + kerNyqstFreq;

    qDebug() << "llim rlim" << llim << rlim;

    //check sizes
    if(freqs.size()+1 != gains.size())
        return false;

    //check frequencies
    double prevFreq = llim;
    for(auto freq : freqs){
        if(freq <= prevFreq)
            return false;
        prevFreq = freq;
    }
    if(!freqs.empty() && freqs.back() >= rlim)
        return false;

    //check gains
    for(auto gain : gains){
        if(gain < 0)
            return false;
    }
    return true;
}

bool GroupSpecs::eventFilter(QObject* obj, QEvent* event){
    if (event->type() == QEvent::PaletteChange && obj == this)
        updatePalette();


    return QWidget::eventFilter(obj, event);
}
