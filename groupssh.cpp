#include <QtWidgets>
#include <string>
#include <QString>
#include <QDebug>
#include "groupssh.h"


GroupSsh::GroupSsh(QWidget *parent) :QGroupBox(tr("SSH options"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    /*>>>Layouts<<<*/

    QVBoxLayout* vBox = new QVBoxLayout;
    QHBoxLayout* idHBox = new QHBoxLayout;
    QGridLayout* commandGrid = new QGridLayout;

    /*>>>ID H Box<<<*/

    QLabel* idLabel = new QLabel(tr("ID"));
    idLabel->setToolTip(tr("Last 6 symbols of Red Pitaya MAC adrress (see WLAN connector)"));
    idLineEdit = new IMLineEdit;
    idLineEdit->setToolTip(tr("Last 6 symbols of Red Pitaya MAC adrress (see WLAN connector)"));
    idLineEdit->setInputMask("HHHHHH");
    //idLineEdit->setMaximumWidth(90);
    QPushButton* connectButton = new QPushButton(tr("Connect"));
    connectButton->setEnabled(false);

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

    //---> Functionality <---//

    connect(connectButton, &QPushButton::released, this, &GroupSsh::onConnect);
    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){connectButton->setEnabled((str.length() == 6));});
//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){idLineEdit->setText(str);idLineEdit->setCursorPosition(str.length());});
//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){qDebug() << str;});

}

void GroupSsh::onConnect(){
    std::string rpMac = idLineEdit->text().toLower().toStdString();
    qDebug() << QString::fromStdString(rpMac);
}
