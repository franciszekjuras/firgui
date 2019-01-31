#include <QtWidgets>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>
#include "groupspecs.h"
#include "firker.h"

GroupSpecs::GroupSpecs(QWidget *parent) :QGroupBox(tr("Filter specification"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    //---> Specification form <---//

    QGridLayout* specsGrid = new QGridLayout;
    freqsLineEdit = new QLineEdit;
    gainsLineEdit = new QLineEdit;
    specsGrid->addWidget(new QLabel(tr("Frequencies")),0,0);
    specsGrid->addWidget(freqsLineEdit, 0, 1);

    QComboBox* unitCombo = new QComboBox;
    unitCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    specsGrid->addWidget(unitCombo, 0, 2);

    specsGrid->addWidget(new QLabel(tr("Gains")),1,0);
    specsGrid->addWidget(gainsLineEdit, 1, 1);
    vBox->addLayout(specsGrid);

    QComboBox* wndCombo = new QComboBox;
    specsGrid->addWidget(new QLabel(tr("Window")),2,0);
    specsGrid->addWidget(wndCombo, 2, 1);

    bandCombo = new QComboBox;
    specsGrid->addWidget(new QLabel(tr("Band")),3,0);
    specsGrid->addWidget(bandCombo, 3, 1);

    //---> Buttons <---//
    QHBoxLayout* buttonsHBox = new QHBoxLayout;
    buttonsHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Fixed));
    QPushButton* calcButton = new QPushButton(tr("Calculate"));
    buttonsHBox->addWidget(calcButton);
    QPushButton* setButton = new QPushButton(tr("Set"));
    buttonsHBox->addWidget(setButton);
    vBox->addLayout(buttonsHBox);


    this->setLayout(vBox);

    //---> Functionality <---//

    unitMult = 1.;

    connect(calcButton, &QPushButton::released, this, &GroupSpecs::calculateKernel);
    connect(this, &GroupSpecs::enableCalcButton, calcButton,  &QPushButton::setEnabled);
    connect(this, &GroupSpecs::enableSetButton, setButton,  &QPushButton::setEnabled);
    enableCalcButton(false); enableSetButton(false);

    connect(this, &GroupSpecs::srcKernelClear, [=](){srcKernelReady(false);});
    connect(this, &GroupSpecs::kernelClear, [=](){kernelReady(false);});

    connect(this, &GroupSpecs::srcKernelChanged, [=](const FirKer& firker){srcKernelReady(true);});
    connect(this, &GroupSpecs::kernelChanged, [=](const FirKer& firker){kernelReady(true);});

    connect(this, &GroupSpecs::resetPlot, [=](double f, int t, int b){srcKernelReady(false);kernelReady(false);});

    connect(unitCombo, &QComboBox::currentTextChanged, this, &GroupSpecs::unitChanged);
    unitCombo->addItem("kHz"); unitCombo->addItem("MHz");

    connect(wndCombo, &QComboBox::currentTextChanged, this, &GroupSpecs::wndChanged);
    QStringList windows;
    windows << tr("None") << tr("Hamming") << tr("Blackman");
    wndCombo->addItems(windows);

    connect(bandCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GroupSpecs::bandChanged);

}

void GroupSpecs::wndChanged(QString wndStr){
    if(wndStr == tr("None"))
        crrWnd = LeastSqFirKer::Window::none;
    else if(wndStr == tr("Hamming"))
        crrWnd = LeastSqFirKer::Window::hamming;
    else if(wndStr == tr("Blackman"))
        crrWnd = LeastSqFirKer::Window::blackman;
    else
        assert(false);
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
    //---> add various bands handling (assume band 0 for now)
    LeastSqFirKer ker;
    double kerSampFreq = fpgaSampFreq / static_cast<double>(t);
    ker.setSampFreq(kerSampFreq);
    ker.setRank(t * d);
    std::vector<double> freqs, gains;

    if(!textToDoubles(freqsLineEdit->text().toStdString(),freqs) ||
        !textToDoubles(gainsLineEdit->text().toStdString(),gains)){
        qDebug()<<"Parsing failed";
        return;
    }
    //---> add band shift and reversal
    int band = bandCombo->currentIndex();
    double kerNqFreq = kerSampFreq / 2.;
    if((band%2) == 1){
        for(auto& v : freqs){
            v -= (kerNqFreq*band); //shift
            v = kerNqFreq - v; //reversal
            qDebug() << v;
        }
        std::reverse(freqs.begin(),freqs.end());
    }

    if(!ker.setSpecs(freqs, gains)){
        qDebug()<<"Bad specs";
        return;
    }
    ker.setWindow(crrWnd);
    if(!ker.calc()){
        qDebug()<<"Calculation failed";
        return;
    }
    crrKer = ker.getKernel();
    emit kernelChanged(ker);
}

void GroupSpecs::calcSrcKernel(){
    //---> add various bands handling (assume band 0 for now)
    // normalized frequency specification -- to avoid double conversion
    if(t == 1) return;

    std::vector<double> freqs, gains, weights;
    int band = bandCombo->currentIndex(); assert (band >= 0); assert (band < t);
    double dT = static_cast<double>(t);
    int srcKerRank = t * s;
    double width = 4.11 / static_cast<double>(srcKerRank); // magic numbers for .1% rippling in passband
    double stopBandWeight = 10.;                           // and -80 dB (.01%) attenuation in stopband

    if(band > 0){
        if(width > .5/dT/2.) return;

        freqs.push_back(.5/dT*band); freqs.push_back(.5/dT*band + width);
        gains.push_back(0); gains.push_back(0);
        weights.push_back(stopBandWeight);
    }
    else if(width > .5/dT) return;

    gains.push_back(1); gains.push_back(1);
    weights.push_back(1.);

    if((band + 1) < t){
        freqs.push_back(.5/dT*(band+1) - width); freqs.push_back(.5/dT*(band+1));
        gains.push_back(0); gains.push_back(0);
        weights.push_back(stopBandWeight);
    }

    EqRippleFirKer ker;
    ker.setSampFreq(1.);
    ker.setRank(srcKerRank);

    if(!ker.setSpecs(freqs,gains,weights)){
        qDebug() << "Wrong EqRipple Filter specs."; return;
    }
    if(!ker.calc()){
        qDebug()<< "SRC kernel calculation failed."; return;
    }
    crrSrcKer = ker.getKernel();
    ker.setSampFreq(fpgaSampFreq);
    emit srcKernelChanged(ker);
}



void GroupSpecs::bitstreamChanged(QMap<QString, int> specs){
    filterReady(false);

    bool validT = (specs.contains("t") && specs["t"] > 0);
    bool validD = (specs.contains("d") && specs["d"] > 0);
    bool validS = (specs.contains("s") && specs["s"] > 0);

    int tOld = t; t = validT ? specs["t"] : 0;
    int dOld = d; d = validD ? specs["d"] : 0;
    int sOld = s; s = validS ? specs["s"] : 0;

    if(!validT){
       bandCombo->clear(); enableCalcButton(false); resetPlot(fpgaSampFreq,1,0);return;
    }

    if(t != tOld){
        rebuild(); //---> this will rebuild all kernels and plot
    }
    else{//---> if t hasn't changed -- test what must be updated
        if(!validS)
            srcKernelClear();
        else if(s != sOld)
            calcSrcKernel();

        if(!validD || d!= dOld)
            kernelClear();
    }

    enableCalcButton(validD);

    //---> finished
}

void GroupSpecs::rebuild(){
    bandCombo->clear();
    double bandW = fpgaSampFreq / t / 2.;
    QString unit(" kHz");
    for(double i = 0.; i < static_cast<double>(t); ++i){
        if(i*bandW >= 1000.){
            bandW /= 1000.;
            unit = " MHz";
        }
        bandCombo->addItem(QString::number(i * bandW) + " - " + QString::number((i+1) * bandW) + unit);
    }
    calcSrcKernel();
}

void GroupSpecs::bandChanged(int band){
    qDebug() << band;
    if(band < 0) return;
    resetPlot(fpgaSampFreq, t, band);
    calcSrcKernel();
}

void GroupSpecs::bitstreamLoaded(QMap<QString, int> specs){
    bitstreamChanged(specs);
    if(isSrcKernelReady)
    {};//send kernel -- if calculation failed, this will prevent loading filter
    //v1: send calculated kernel
    //v2: if ready send kernel
    //    else set awaitingSRCKernel = true
}

void GroupSpecs::filterReady(bool en){
    isFilterReady = en;
    enableSetButton(isFilterReady && isKernelReady);
}

void GroupSpecs::kernelReady(bool en){
    isKernelReady = en;
    enableSetButton(isFilterReady && isKernelReady);
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
