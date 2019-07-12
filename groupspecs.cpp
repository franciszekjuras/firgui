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

#define PASSBAND_WIDTH_RATIO 4.11 // magic numbers for .1% rippling in passband
                                  // and -80 dB (.01%) attenuation in stopband

GroupSpecs::GroupSpecs(QWidget *parent) :QGroupBox(tr("Filter specification"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    //---> Specification form <---//

    QGridLayout* specsGrid = new QGridLayout;
    freqsLineEdit = new QLineEdit;
    gainsLineEdit = new QLineEdit;
    freqsLineEdit->setToolTip(tr("Space separated corner frequencies (in band range)."));
    gainsLineEdit->setToolTip(tr("Space separated gains (1 for passband, 0 for stopband)."));
    specsGrid->addWidget(new QLabel(tr("Frequencies")),0,0);
    specsGrid->addWidget(freqsLineEdit, 0, 1);

    QComboBox* unitCombo = new QComboBox;
    unitCombo->setFocusPolicy(Qt::ClickFocus);
    unitCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    specsGrid->addWidget(unitCombo, 0, 2);

    specsGrid->addWidget(new QLabel(tr("Gains")),1,0);
    specsGrid->addWidget(gainsLineEdit, 1, 1);
    vBox->addLayout(specsGrid);

    QComboBox* wndCombo = new QComboBox;
    wndCombo->setToolTip("Different windows give various roll-off/rippling tradeoffs.");
    specsGrid->addWidget(new QLabel(tr("Window")),2,0);
    specsGrid->addWidget(wndCombo, 2, 1);

    bandCombo = new QComboBox;
    specsGrid->addWidget(new QLabel(tr("Working Band")),3,0);
    specsGrid->addWidget(bandCombo, 3, 1);

    //---> Buttons <---//
    QHBoxLayout* buttonsHBox = new QHBoxLayout;


    QPushButton* helpButton = new QPushButton(tr("Help"));
    helpButton->setFocusPolicy(Qt::ClickFocus);
    QFontMetrics fm = helpButton->fontMetrics();
    helpButton->setMaximumWidth(fm.width(tr("Help"))+20);
    buttonsHBox->addWidget(helpButton);

    buttonsHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Fixed));

    waitSpin = new WaitingSpinnerWidget(0, false, false);
    buttonsHBox->addWidget(waitSpin);
    QPushButton* calcButton = new QPushButton(tr("Calculate"));
    buttonsHBox->addWidget(calcButton);
    QPushButton* setButton = new QPushButton(tr("Set"));
    buttonsHBox->addWidget(setButton);
    vBox->addLayout(buttonsHBox);


    this->setLayout(vBox);


    //---> Functionality <---//
    waitSpin->setRoundness(70.0);
    waitSpin->setMinimumTrailOpacity(50.0);
    waitSpin->setTrailFadePercentage(70.0);
    waitSpin->setNumberOfLines(10);
    waitSpin->setLineLength(6);
    waitSpin->setLineWidth(3);
    waitSpin->setInnerRadius(5);
    waitSpin->setRevolutionsPerSecond(2);
    waitSpin->setColor(QColor(0, 150, 136));
    //waitSpin->setColor(QColor(137, 207, 240));


    unitMult = 1.;

    connect(freqsLineEdit, &QLineEdit::returnPressed, gainsLineEdit, static_cast<void (QLineEdit::*)()>(&QLineEdit::setFocus));
    connect(gainsLineEdit, &QLineEdit::returnPressed, calcButton, &QPushButton::click);

    connect(helpButton, &QPushButton::released, this, &GroupSpecs::showHelp);

    connect(calcButton, &QPushButton::released, this, &GroupSpecs::calculateKernel);
    connect(this, &GroupSpecs::enableCalcButton, calcButton,  &QPushButton::setEnabled);
    connect(setButton, &QPushButton::released, this, &GroupSpecs::setKernels);
    connect(this, &GroupSpecs::enableSetButton, setButton,  &QPushButton::setEnabled);
    enableCalcButton(false); enableSetButton(false);

    connect(this, &GroupSpecs::srcKernelClear, [=](){srcKernelReady(false);});
    connect(this, &GroupSpecs::kernelClear, [=](){kernelReady(false);});

    connect(this, &GroupSpecs::srcKernelChanged, [=](std::shared_ptr<const FirKer> firker){srcKernelReady(true);});
    connect(this, &GroupSpecs::kernelChanged, [=](std::shared_ptr<const FirKer> firker){kernelReady(true);});

    connect(this, &GroupSpecs::resetPlot, [=](double f, int t, int b){srcKernelReady(false);kernelReady(false);});

    connect(unitCombo, &QComboBox::currentTextChanged, this, &GroupSpecs::unitChanged);
    unitCombo->addItem("kHz"); unitCombo->addItem("MHz");

    connect(wndCombo, &QComboBox::currentTextChanged, this, &GroupSpecs::wndChanged);
    QStringList windows;
    windows << tr("None") << tr("Hamming") << tr("Blackman");
    wndCombo->addItems(windows);
    wndCombo->setCurrentText(tr("Hamming"));

    connect(bandCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GroupSpecs::bandChanged);


    connect(&spinWatch, &BoolMapOr::valueChanged, [=](bool v){if(v)this->waitSpin->start(); else this->waitSpin->stop();});

    //qRegisterMetaType< std::shared_ptr<FirKer> >("FirKer");
//    connect(&kerCalcThread, &KernelCalcThread::started, this, [=](){this->waitSpin->start();});
//    connect(&kerCalcThread, &KernelCalcThread::calcFinished, this, &GroupSpecs::kerCalcFinished);
    connect(&kerCalcWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::started, this, [=](){spinWatch.enable("kerCalc");});
    connect(&kerCalcWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::finished, this, &GroupSpecs::kerCalcFinished);

//    connect(&srcKerCalcThread, &KernelCalcThread::started, this, [=](){this->waitSpin->start();});
//    connect(&srcKerCalcThread, &KernelCalcThread::calcFinished, this, &GroupSpecs::srcKerCalcFinished);
    connect(&srcKerCalcWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::started, this, [=](){spinWatch.enable("srcKerCalc");});
    connect(&srcKerCalcWatch, &QFutureWatcher<std::shared_ptr<FirKer>>::finished, this, &GroupSpecs::srcKerCalcFinished);
    pendCalcSrcKernel = false;
    pendCalculateKernel = false;
    kerLocked = false;
    srcKerLocked = false;
    isKernelReady = false;
    isSrcKernelReady = false;
    isFilterReady = false;
    isSrcKernelLoaded = false;

}

void GroupSpecs::handleConnect(bool is){
    if(!is){
        filterReady(false);
        isSrcKernelLoaded = false;
    }
}

void GroupSpecs::showHelp(){
    QMessageBox::information(this, tr("FIR Controller -- Help"), tr("    Filter is described by specifing gains to subsequent frequency ranges in the given band (which is configurable). Outside of this band gain is always zero (nothing is passed).\nCorner frequencies should be given omitting boundary frequencies with gains matching successive ranges.\n\n    For example: with filter band from 0 to 500 kHz, filter passing signal from 0 to 200 kHz, stopping everything from 200 to 400 kHz and passing with amplitude divided by 2 from 400 to 500 kHz, can be set by giving:\n\nFrequencies: 200 400\nGains: 1 0 0.5\n\n    Note however, that overall filtering result is affected by rate conversion transmission (shown on plot) and signal may not be passed on band boundaries.\n\n    For further details read tooltips, which can be displayed by hovering the cursor over an object."),tr("Close"));
}

void GroupSpecs::wndChanged(QString wndStr){
    if(wndStr == tr("None"))
        crrWnd = LeastSqFirKer::Window::none;
    else if(wndStr == tr("Hamming"))
        crrWnd = LeastSqFirKer::Window::hamming;
    else if(wndStr == tr("Blackman"))
        crrWnd = LeastSqFirKer::Window::blackman;
    else
        assert(wndStr.isEmpty());
}

void GroupSpecs::unitChanged(QString unit){
    double newUnitMult;
    if(unit == "kHz")
        newUnitMult = 1.;
    else if(unit == "MHz")
        newUnitMult = 1000.;
    else
        assert(false);

    std::vector<double> freqs;

    if(newUnitMult != unitMult && textToDoubles(freqsLineEdit->text().toStdString(),freqs)){
        double ratio = unitMult / newUnitMult;
        QString newFreqs;
        for(auto& v : freqs){
            v *= ratio;
            newFreqs += QString::number(v) += " ";
        }
        freqsLineEdit->setText(newFreqs);
    }
    unitMult = newUnitMult;
}

void GroupSpecs::calculateKernel(){
    kernelReady(false);
    if(kerCalcWatch.isRunning()){pendCalculateKernel = true; return;}
    pendCalculateKernel = false;

    LeastSqFirKer ker;
    double kerSampFreq = fpgaSampFreq / static_cast<double>(t);
    ker.setSampFreq(kerSampFreq);
    ker.setRank(t * d);
    std::vector<double> freqs, gains;

    if(!textToDoubles(freqsLineEdit->text().toStdString(),freqs) ||
        !textToDoubles(gainsLineEdit->text().toStdString(),gains)){
        return;
    }

    //unit conversion
    for(auto& v : freqs){
        v *= unitMult;
    }

    int band = currentBand();
    double kerNqFreq = kerSampFreq / 2.;

    for(auto& v : freqs)
        v -= (kerNqFreq*band); //shift

    if((band%2) == 1){
        //band reversal
        for(auto& v : freqs)
            v = kerNqFreq - v; //reversal

        std::reverse(freqs.begin(),freqs.end());
        std::reverse(gains.begin(),gains.end());
    }

    if(!ker.setSpecs(freqs, gains)){
        int uc = QMessageBox::warning(this, tr("FIR Controller - Incorrect Specification"),tr("Incorrect filter specification. Example:\n\nFrequencies: 200 300 (kHz)\nGains: 0 1 0\nBand: 0 - 500 kHz"),tr("Help"),tr("Close"),QString(),1, 1);
        if(uc == 0)showHelp();
        return;
    }
    ker.setWindow(crrWnd);

    std::shared_ptr<FirKer> ker_shared = std::make_shared<LeastSqFirKer>(ker);
    QFuture<std::shared_ptr<FirKer>> fut = QtConcurrent::run([=](){ker_shared->calc(); return ker_shared;});
    kerCalcWatch.setFuture(fut);
}

void GroupSpecs::kerCalcFinished(){
    spinWatch.disable("kerCalc");
    //kerLocked = false;
    if(pendCalculateKernel){calculateKernel();return;}

    std::shared_ptr<FirKer> ker = kerCalcWatch.future().result();

    if(!ker->isValid()){
        int uc = QMessageBox::warning(this, tr("FIR Controller - Incorrect Specification"),tr("Incorrect filter specification. Example:\n\nFrequencies: 200 300 (kHz)\nGains: 0 1 0\nBand: 0 - 500 kHz"),tr("Help"),tr("Close"),QString(),1, 1);
        if(uc == 0)showHelp();
        return;
    }
    crrKer = ker->getKernel();
    emit kernelChanged(ker);
}

void GroupSpecs::calcSrcKernel(){
    kernelReady(false); isSrcKernelLoaded = false;

    if(srcKerCalcWatch.isRunning()){
        pendCalcSrcKernel = true; return;
    }
    pendCalcSrcKernel = false;
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
        qDebug() << "Band" << band << "t" << t;
        qDebug() << tr("Higher bands are not supported in this configuration");
        return;
    }

    if(band > 0){
        gains.push_back(0); // 0 frequency
        freqs.push_back(.5/dT*(dband + .0001)); gains.push_back(0); // small shift added to prevent kernel calculation issues
        freqs.push_back(.5/dT*dband + width); // max frequency
        weights.push_back(stopBandWeight);
    }
    else if(width > .5/dT) return;

    gains.push_back(1); gains.push_back(1);
    weights.push_back(1.);

    if((band + 1) < t){
        freqs.push_back(.5/dT*(dband+1.) - width); freqs.push_back(.5/dT*(dband+1.-.0001));
        gains.push_back(0); gains.push_back(0);
        weights.push_back(stopBandWeight);
    }

    EqRippleFirKer ker;
    ker.setSampFreq(1.);
    ker.setRank(srcKerRank);

    if(!ker.setSpecs(freqs,gains,weights)){
        qDebug() << "Wrong EqRipple Filter specs."; return;
    }

    std::shared_ptr<FirKer> ker_shared = std::make_shared<EqRippleFirKer>(ker);
    QFuture<std::shared_ptr<FirKer>> fut = QtConcurrent::run([=](){ker_shared->calc(); return ker_shared;});
    srcKerCalcWatch.setFuture(fut);
}

void GroupSpecs::srcKerCalcFinished(){
    spinWatch.disable("srcKerCalc");
    if(pendCalcSrcKernel){calcSrcKernel();return;}


    std::shared_ptr<FirKer> ker = srcKerCalcWatch.future().result();
    if(!ker->isValid()){qDebug()<<"Src kernel calculation failed.";return;}

    crrSrcKer = ker->getKernel();
    while(crrSrcKer.size() < ker->getRank()){
        qDebug() << "Adding 0 to src kernel.";
        crrSrcKer.push_back(0.);
    }
    ker->setSampFreq(fpgaSampFreq);
    emit srcKernelChanged(ker);
}

void GroupSpecs::setKernels(){
    if(!isSrcKernelLoaded)
        reqLoadSrcKernel(crrSrcKer);
    isSrcKernelLoaded = true;
    reqLoadKernel(crrKer);
}

void GroupSpecs::bitstreamChanged(QMap<QString, int> specs){
    filterReady(false);

    bool validT = (specs.contains("tm") && specs["tm"] > 0);
    bool validD = (specs.contains("fb") && specs["fb"] > 0);
    bool validS = (specs.contains("sb") && specs["sb"] > 0);

    int tOld = t; t = validT ? specs["tm"] : 0;
    int dOld = d; d = validD ? specs["fb"] : 0;
    int sOld = s; s = validS ? specs["sb"] : 0;

    if(!validT){
       bandCombo->clear(); enableCalcButton(false); resetPlot(fpgaSampFreq,1,0);return;
    }
    //TODO: find better solution (to prevent clearing calculated kernel when loading bitstream)
    if(t == tOld && validS && s == sOld && validD && d == dOld)
        qDebug() << "No changes in bitstream.";
    else
        rebuild();

//    if(t != tOld){
//        rebuild(); //---> this will rebuild all kernels and plot
//    }
//    else{//---> if t hasn't changed -- test what must be updated
//        if(!validS)
//            srcKernelClear();
//        else if(s != sOld)
//            calcSrcKernel();

//        if(!validD || d!= dOld)
//            kernelClear();
//    }

    enableCalcButton(validD);

    //---> finished
}

void GroupSpecs::rebuild(){
    bandCombo->clear();
    double bandW = fpgaSampFreq / t / 2.;
    QString unit(" kHz");


    double dT = static_cast<double>(t);
    int srcKerRank = t * s;
    double width = PASSBAND_WIDTH_RATIO / static_cast<double>(srcKerRank);
    middleBandsEn = (width < .5/dT/2.);

    for(double i = 0.; i < dT; ++i){
        if((i+1)*bandW >= 1000.){
            bandW /= 1000.;
            unit = " MHz";
        }
        //QString::asprintf("%+06.2f", value)
        if(middleBandsEn || i == 0. || i == (dT -1))
        bandCombo->addItem(QString::number(i * bandW,'g',4) + " - " + QString::number((i+1) * bandW,'g',4) + unit);
    }
}

void GroupSpecs::bandChanged(int band){
    if(band < 0) return;
    resetPlot(fpgaSampFreq, t, currentBand());
    calcSrcKernel();
}

void GroupSpecs::bitstreamLoaded(QMap<QString, int> specs){
    bitstreamChanged(specs);
    filterReady(true);
}

void GroupSpecs::filterReady(bool en){
    isFilterReady = en;
    enableSetButton(isFilterReady && isKernelReady && isSrcKernelReady);
    //qDebug() << "filterReady" << isFilterReady << isKernelReady << isSrcKernelReady;
}

void GroupSpecs::kernelReady(bool en){
    isKernelReady = en;
    enableSetButton(isFilterReady && isKernelReady && isSrcKernelReady);
    //qDebug() << "kernelReady" << isFilterReady << isKernelReady << isSrcKernelReady;
}

void GroupSpecs::srcKernelReady(bool en){
    isSrcKernelReady = en;
    enableSetButton(isFilterReady && isKernelReady && isSrcKernelReady);
    //qDebug() << "srcKernelReady" << isFilterReady << isKernelReady << isSrcKernelReady;
}

void GroupSpecs::setFpgaSampFreq(double freq){
    fpgaSampFreq = freq;
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
