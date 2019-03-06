#include <QtWidgets>
#include <string>
#include <QString>
#include <QDebug>
#include <QShortcut>
#include "groupssh.h"
#include "switch.h"


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
    idLineEdit->setFixedWidth(this->fontMetrics().width("HHHHHH") + 10);
    //idLineEdit->setMaximumWidth(90);
    QPushButton* connectButton = new QPushButton(tr("Connect"));
    connectButton->setEnabled(false);

    idHBox->addWidget(idLabel);
    idHBox->addWidget(idLineEdit);
    idHBox->addWidget(connectButton);
    idHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));

    //QSpacerItem* scSpacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred);

    /*>>>Advanced Options<<<*/

    //QSpacerItem* advSpacer = new QSpacerItem(0, 10);
    advButton = new Switch(tr("Advanced Options"));
    enableAdvState = true; toggleEnableAdv();


    //advButton->setCheckable(true);

    /*>>>Command Grid<<<*/

    QLineEdit* commandLine = new QLineEdit;
    QPushButton* commandButton = new QPushButton(tr("Send"));
    QFontMetrics fm = commandButton->fontMetrics();
    commandButton->setMaximumWidth(fm.width(tr("Send"))+20);
    QTextBrowser* commandBrowser = new QTextBrowser;
    commandBrowser->setFixedHeight(70);

    commandGrid->addWidget(commandLine, 0, 0);
    commandGrid->addWidget(commandButton, 0, 1);
    commandGrid->addWidget(commandBrowser, 1, 0, 1, 2);

    //set visibility toogle
    commandBrowser->setVisible(false);commandLine->setVisible(false);commandButton->setVisible(false);
    connect(advButton, SIGNAL(toggled(bool)), commandBrowser, SLOT(setVisible(bool)));
    connect(advButton, SIGNAL(toggled(bool)), commandLine, SLOT(setVisible(bool)));
    connect(advButton, SIGNAL(toggled(bool)), commandButton, SLOT(setVisible(bool)));



    /*>>>Widget Layout<<<*/

    vBox->addLayout(idHBox);
    //vBox->addItem(advSpacer);
    vBox->addWidget(advButton);
    vBox->addLayout(commandGrid);
    //vBox->addWidget(responseTextBox);
    this->setLayout(vBox);

    //---> Functionality <---//

    connect(connectButton, &QPushButton::released, this, &GroupSsh::onConnect);
    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){connectButton->setEnabled((str.length() == 6));});
//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){idLineEdit->setText(str);idLineEdit->setCursorPosition(str.length());});
//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){qDebug() << str;});
    QShortcut* sc = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
    sc->setContext(Qt::ApplicationShortcut);
    connect(sc, &QShortcut::activated, this, &GroupSsh::toggleEnableAdv );

    qDebug()<<"Calling libssh";
    ssh.setUser("root");
    ssh.setHost("localhost");
    ssh.connect();

}

void GroupSsh::onConnect(){
    std::string rpMac = idLineEdit->text().toLower().toStdString();
    qDebug() << QString::fromStdString(rpMac);
}

void GroupSsh::toggleEnableAdv(){
    enableAdvState = !enableAdvState;
    if(advButton->isChecked())
        advButton->animateClick(0);
    advButton->setEnabled(enableAdvState);
}
