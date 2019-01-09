#include <QtWidgets>
#include "groupconfig.h"

GroupConfig::GroupConfig(QWidget *parent) : QGroupBox(tr("Configuration"),parent)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QVBoxLayout* vBox = new QVBoxLayout;

    QFormLayout* bitForm = new QFormLayout;
    this->bitCombo = new QComboBox;
    bitForm->addRow(tr("Bitstream"), this->bitCombo);
    vBox->addLayout(bitForm);


    this->setLayout(vBox);
}

