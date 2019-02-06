#include <QtWidgets>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QMap>
#include <cassert>
#include "groupconfig.h"
#include "bitstreamspecs.h"

GroupConfig::GroupConfig(QWidget *parent) : QGroupBox(tr("Configuration"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    QFormLayout* bitForm = new QFormLayout;
    bitMainCombo = new QComboBox;
    bitForm->addRow(tr("Nyquist frequency"), bitMainCombo);
    bitSpecCombo = new QComboBox;
    QString specComboName = tr("Filter Rank") + " | " + tr("SRC blocks");
    bitForm->addRow(specComboName, bitSpecCombo);
    vBox->addLayout(bitForm);

    QHBoxLayout* buttonHBox = new QHBoxLayout;
    buttonHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred));
    QPushButton* loadButton = new QPushButton(tr("Load"));
    buttonHBox->addWidget(loadButton);
    vBox->addLayout(buttonHBox);

    this->setLayout(vBox);

    //---> Functionality <---//

//    for(auto it = bitMap.cbegin(); it != bitMap.cend(); ++it)
//        qDebug() << it.key() << " " << it.value().getFileName();

//    BitstreamSpecs bitspecs("fir3v11_t125d39s20fm21cm20.bin");
//    qDebug() << "Valid:" << bitspecs.isValid();
//    qDebug() << "Name:" << bitspecs.getFilename();
//    qDebug() << "Version:" << bitspecs.getVersion();
//    qDebug() << "Specs:";
//    QMap<QString, int> specs = bitspecs.getSpecs();
//    for(auto it = specs.cbegin(); it != specs.cend(); ++it)
//        qDebug() << it.key() << " " << it.value();

//    qDebug() << "Comment:" << bitspecs.getComment();

//    qDebug() << "Sampling division:" << specs["t"];

//    QFileInfo file("bitstreams/lol.txt");
//    qDebug() << "1:" << file.fileName();
//    qDebug() << "2:" << file.filePath();
//    qDebug() << "3:" << file.path();

}

void GroupConfig::init(){


    fpgaSampFreq = 125000.;
    emit fpgaSampFreqChanged(fpgaSampFreq);

    QDir bitDir("bitstreams");
    bitDir.setFilter(QDir::Files);
    bitDir.setNameFilters(QStringList("*.bin"));
    qDebug() << "bitstreams dir exists:"<< bitDir.exists();
    QFileInfoList binFiles = bitDir.entryInfoList();


    for(const auto& fI : binFiles){
        BitstreamSpecs bitSpec(fI);
        auto params = bitSpec.getSpecs();
        if(bitSpec.isValid() && params.contains("t")){
            double nyqFreq = fpgaSampFreq/2./ static_cast<double>(params["t"]);
            QString key1;
            if(nyqFreq >= 1000.) key1 = QString::number(nyqFreq/1000.,'g',4)+" MHz";
            else key1 = QString::number(nyqFreq,'g',4)+" KHz";
            QString key2;

            if(params.contains("d"))
                key2 += QString::number(params["d"]*params["t"]);
            else
                key2 += tr("unk.");

            key2 += " | ";

            if(params.contains("s"))
                key2 += QString::number(params["s"]);
            else
                key2 += tr("unk.");

            bitMap[key1][key2] = bitSpec;
        }
    }

    connect(bitMainCombo, &QComboBox::currentTextChanged, this, &GroupConfig::bitMainComboChanged);
    connect(bitSpecCombo, &QComboBox::currentTextChanged, this, &GroupConfig::bitSpecComboChanged);

    for(auto it = bitMap.cbegin(); it != bitMap.cend(); ++it){
        qDebug()<<"it:"<<it.key();
        bitMainCombo->addItem(it.key());
    }
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
    emit bitstreamSelected(bitMap[bitMainStr][specStr].getSpecs());
}
