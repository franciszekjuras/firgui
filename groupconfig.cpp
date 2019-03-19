#include <QtWidgets>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QMap>
#include <vector>
#include <algorithm>
#include <cassert>
#include "groupconfig.h"
#include "bitstreamspecs.h"

GroupConfig::GroupConfig(QWidget *parent) : QGroupBox(tr("Configuration"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    QFormLayout* bitForm = new QFormLayout;
    bitMainCombo = new QComboBox;
    bitMainCombo->setToolTip("Roll-off decreases qudratically with working band width.");
    bitForm->addRow(tr("Working band width"), bitMainCombo);
    bitSpecCombo = new QComboBox;
    bitSpecCombo->setToolTip(tr("Higher number of SRC blocks gives sharper rate conversion transmission on cost of reduced filter rank."));
    QString specComboName = tr("Filter Rank") + " | " + tr("SRC blocks");
    bitForm->addRow(specComboName, bitSpecCombo);
    vBox->addLayout(bitForm);

    QHBoxLayout* buttonHBox = new QHBoxLayout;
    buttonHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred));
    QPushButton* loadButton = new QPushButton(tr("Load"));
    loadButton->setDisabled(true);
    buttonHBox->addWidget(loadButton);
    vBox->addLayout(buttonHBox);

    this->setLayout(vBox);

    //---> Functionality <---//

    connect(this, &GroupConfig::enableLoad, loadButton, &QPushButton::setEnabled);
    connect(loadButton, &QPushButton::released, this, &GroupConfig::onLoadButton);

//    QFileInfo file("bitstreams/lol.txt");
//    qDebug() << "1:" << file.fileName();
//    qDebug() << "2:" << file.filePath();
//    qDebug() << "3:" << file.path();

}

void GroupConfig::onLoadButton(){
    reqLoad(crrBitstream);
}

void GroupConfig::init(){


    fpgaSampFreq = 125000.;
    emit fpgaSampFreqChanged(fpgaSampFreq);

    QDir bitDir("data/bitstreams");
    bitDir.setFilter(QDir::Files);
    bitDir.setNameFilters(QStringList("*.bin"));
    qDebug() << "bitstreams dir exists:"<< bitDir.exists();
    QFileInfoList binFiles = bitDir.entryInfoList();

    std::vector<BitstreamSpecs> bitSpecsV;
    for(const auto& fn : binFiles){
        BitstreamSpecs bitSpec(fn);
        if(bitSpec.isValid() && bitSpec.getSpecs().contains("tm"))
            bitSpecsV.push_back(bitSpec);
    }

    auto bitSortF = [](const BitstreamSpecs& a, const BitstreamSpecs& b){
        int atm = a.getSpecs().value("tm",0);
        int btm = b.getSpecs().value("tm",0);
        if(atm < btm) return true;
        if(atm > btm) return false;
        int afb = a.getSpecs().value("fb",0);
        int bfb = b.getSpecs().value("fb",0);
        if(afb < bfb) return true;
        return false;
    };

    std::sort(bitSpecsV.begin(),bitSpecsV.end(), bitSortF);

    QString key1P;
    for(const auto& bitSpec : bitSpecsV){
        auto params = bitSpec.getSpecs();
        double nyqFreq = fpgaSampFreq/2./ static_cast<double>(params["tm"]);
        QString key1;
        if(nyqFreq >= 1000.) key1 = QString::number(nyqFreq/1000.,'g',4)+" MHz";
        else key1 = QString::number(nyqFreq,'g',4)+" KHz";
        QString key2;

        if(params.contains("fb"))
            key2 += QString::number(params["fb"]*params["tm"]);
        else
            key2 += tr("unk.");

        key2 += " | ";

        if(params.contains("sb"))
            key2 += QString::number(params["sb"]);
        else
            key2 += tr("unk.");

        if(key1 != key1P){
            bitMainCombo->addItem(key1);
            key1P = key1;
        }

        bitMap[key1][key2] = bitSpec;
    }

    bitMainCombo->setCurrentIndex(-1);
    connect(bitMainCombo, &QComboBox::currentTextChanged, this, &GroupConfig::bitMainComboChanged);
    connect(bitSpecCombo, &QComboBox::currentTextChanged, this, &GroupConfig::bitSpecComboChanged);
    bitMainCombo->setCurrentIndex(0);



//    for(auto it = bitMap.cbegin(); it != bitMap.cend(); ++it){
//        qDebug()<<"it:"<<it.key();
//        bitMainCombo->addItem(it.key());
//    }
}

void GroupConfig::bitMainComboChanged(QString mainStr){
    assert(bitMap.contains(mainStr));
    updateBitSpecCombo(mainStr);
}

void GroupConfig::updateBitSpecCombo(QString mainStr){
    bitMainStr = mainStr;
    bitSpecCombo->clear();
    auto specMap = bitMap.value(mainStr);
    for(auto it = specMap.cbegin(); it != specMap.cend(); ++it)
        bitSpecCombo->addItem(it.key());
}

void GroupConfig::bitSpecComboChanged(QString specStr){
    if(specStr.isEmpty()) return;
    qDebug()<<"specStr:"<<specStr;
    assert(bitMap.value(bitMainStr).contains(specStr));
    crrBitstream = bitMap[bitMainStr][specStr];
    emit bitstreamSelected(crrBitstream.getSpecs());
}
