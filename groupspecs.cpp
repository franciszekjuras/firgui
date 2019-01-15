#include <QtWidgets>
#include <QDebug>
#include <string>
#include <sstream>
#include "groupspecs.h"
#include "firker.h"

GroupSpecs::GroupSpecs(QWidget *parent) :QGroupBox(tr("Filter specification"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    //---> Specification form <---//

    QFormLayout* specsForm = new QFormLayout;
    freqsLineEdit = new QLineEdit;
    gainsLineEdit = new QLineEdit;
    specsForm->addRow(tr("Frequencies"), freqsLineEdit);
    specsForm->addRow(tr("Gains"), gainsLineEdit);
    vBox->addLayout(specsForm);

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

    connect(calcButton, &QPushButton::released, this, &GroupSpecs::calculateKernel);
    connect(this, &GroupSpecs::enableCalcButton, calcButton,  &QPushButton::setEnabled);
    connect(this, &GroupSpecs::enableCalcButton, this, &GroupSpecs::setCalcEn);
    connect(this, &GroupSpecs::enableSetButton, setButton,  &QPushButton::setEnabled);
    enableCalcButton(false); enableSetButton(false);

}

void GroupSpecs::calculateKernel(){
    EqRippleFirKer ker;
    ker.setSampFreq(kerSampFreq);
    ker.setRank(kerRank);
    std::vector<double> freqs, gains;

    if(!textToDoubles(freqsLineEdit->text().toStdString(),freqs) ||
        !textToDoubles(gainsLineEdit->text().toStdString(),gains)){
        qDebug()<<"Parsing failed";
        return;
    }
    if(!ker.setSpecs(freqs, gains)){
        qDebug()<<"Bad specs";
        return;
    }
    if(!ker.calc()){
        qDebug()<<"Calculation failed";
        return;
    }
    this->crrKer = ker.getKernel();
//    std::vector<double> trns(ker.transmission(100));
//    for(auto v : trns)
//        qDebug()<<v;
    emit kernelChanged(ker);
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

void GroupSpecs::bitstreamChanged(QMap<QString, int> specs){
    if(!(specs.contains("t") && specs.contains("d") && specs.contains("s"))){
        reqClearPlot();
        enableCalcButton(false); enableSetButton(false);
    }
    int newKerRank = specs["t"]*specs["d"];
    double newKerSampFreq = fpgaSampFreq / static_cast<double>(specs["t"]);
    if(newKerRank == kerRank && newKerSampFreq == kerSampFreq)
        return;
    kerRank = newKerRank; kerSampFreq = newKerSampFreq;
    reqClearPlot();
    //calculateKernel();
    enableCalcButton(true);
}

void GroupSpecs::bitstreamLoaded(){
    if(calcEn)
        enableSetButton(true);
}

void GroupSpecs::setFpgaSampFreq(double freq){
    fpgaSampFreq = freq;
}
