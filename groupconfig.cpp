#include <QtWidgets>
#include <QDebug>
#include <QFileInfo>
#include "groupconfig.h"
#include "bitstreamspecs.h"

GroupConfig::GroupConfig(QWidget *parent) : QGroupBox(tr("Configuration"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* vBox = new QVBoxLayout;

    QFormLayout* bitForm = new QFormLayout;
    QComboBox* bitCombo = new QComboBox;
    bitForm->addRow(tr("Bitstream"), bitCombo);
    vBox->addLayout(bitForm);

    QHBoxLayout* buttonHBox = new QHBoxLayout;
    //buttonHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Preferred, QSizePolicy::Preferred));
    QPushButton* loadButton = new QPushButton(tr("Load"));
    loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonHBox->addWidget(loadButton,0,Qt::AlignRight);
    vBox->addLayout(buttonHBox);

    this->setLayout(vBox);

    //---> Functionality <---//
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

    QFileInfo file("bitstreams/lol.txt");
    qDebug() << "1:" << file.fileName();
    qDebug() << "2:" << file.filePath();
    qDebug() << "3:" << file.path();

}

