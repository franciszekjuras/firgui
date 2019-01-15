#include <QtWidgets>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QMap>
#include "groupconfig.h"
#include "bitstreamspecs.h"

GroupConfig::GroupConfig(QWidget *parent) : QGroupBox(tr("Configuration"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    QFormLayout* bitForm = new QFormLayout;
    bitCombo = new QComboBox;
    bitForm->addRow(tr("Nyquist frequency"), bitCombo);
    vBox->addLayout(bitForm);

    QHBoxLayout* buttonHBox = new QHBoxLayout;
    //buttonHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Preferred, QSizePolicy::Preferred));
    QPushButton* loadButton = new QPushButton(tr("Load"));
    loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonHBox->addWidget(loadButton,0,Qt::AlignRight);
    vBox->addLayout(buttonHBox);

    this->setLayout(vBox);

    //---> Functionality <---//







    for(auto it = bitMap.cbegin(); it != bitMap.cend(); ++it)
        qDebug() << it.key() << " " << it.value().getFileName();

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
        if(bitSpec.isValid() && bitSpec.getSpecs().contains("t")){
            double nyqFreq = fpgaSampFreq/2./ static_cast<double>(bitSpec.getSpecs().value("t"));
            QString key;
            if(nyqFreq >= 1000.) key = QString::number(nyqFreq/1000.,'g',4)+" MHz";
            else key = QString::number(nyqFreq,'g',4)+" KHz";
            bitMap[key] = bitSpec;
        }
    }

    connect(bitCombo, &QComboBox::currentTextChanged, this, &GroupConfig::bitComboChanged);

    for(auto it = bitMap.cbegin(); it != bitMap.cend(); ++it)
        bitCombo->addItem(it.key());
}

void GroupConfig::bitComboChanged(QString str){
    if(!bitMap.contains(str)){
        qDebug() << "Error: Unknown bitCombo string."; return;
    }
    emit bitstreamSelected(bitMap.value(str).getSpecs());
}
