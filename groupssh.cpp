#include <QtWidgets>
#include "groupssh.h"



GroupSsh::GroupSsh(QWidget *parent) :QGroupBox(tr("SSH options"),parent)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    /*>>>Layouts<<<*/

    QVBoxLayout* vBox = new QVBoxLayout;
    QHBoxLayout* idHBox = new QHBoxLayout;
    QGridLayout* commandGrid = new QGridLayout;

    /*>>>ID H Box<<<*/

    QLabel* idLabel = new QLabel(tr("ID"));
    idLabel->setToolTip(tr("Last 6 symbols of Red Pitaya MAC adrress (see WLAN connector)"));
    QLineEdit* idLineEdit = new QLineEdit;
    QPushButton* connectButton = new QPushButton(tr("Connect"));

    idHBox->addWidget(idLabel);
    idHBox->addWidget(idLineEdit);
    idHBox->addWidget(connectButton);

    //QSpacerItem* scSpacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred);

    /*>>>Advanced Options<<<*/

    QSpacerItem* advSpacer = new QSpacerItem(0, 10);
    QPushButton* advButton = new QPushButton(tr("Advanced Options"));
    advButton->setCheckable(true);

    /*>>>Command Grid<<<*/

    QLineEdit* commandLine = new QLineEdit;
    QPushButton* commandButton = new QPushButton(tr("Send"));
    QTextBrowser* commandBrowser = new QTextBrowser;

    commandGrid->addWidget(commandLine, 0, 0);
    commandGrid->addWidget(commandButton, 0, 1);
    commandGrid->addWidget(commandBrowser, 1, 0, 1, 2);

    //set visibility toogle

    connect(advButton, SIGNAL(toggled(bool)), commandBrowser, SLOT(setVisible(bool)));
    connect(advButton, SIGNAL(toggled(bool)), commandLine, SLOT(setVisible(bool)));
    connect(advButton, SIGNAL(toggled(bool)), commandButton, SLOT(setVisible(bool)));
    advButton->setChecked(true);advButton->setChecked(false);

    /*>>>Widget Layout<<<*/

    vBox->addLayout(idHBox);
    vBox->addItem(advSpacer);
    vBox->addWidget(advButton);
    vBox->addLayout(commandGrid);
    //vBox->addWidget(responseTextBox);
    this->setLayout(vBox);

}
