#include <QtWidgets>
#include "groupconfig.h"

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
    QPushButton* loadButton = new QPushButton("Load");
    loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonHBox->addWidget(loadButton,0,Qt::AlignRight);
    vBox->addLayout(buttonHBox);


    this->setLayout(vBox);
}

