#include <QtWidgets>
#include <string>
#include <QString>
#include <QDebug>
#include <QShortcut>
#include <QInputDialog>
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
    connectButton = new QPushButton(tr("Connect"));
    connectButton->setEnabled(false);

    disconnectButton = new QPushButton(tr("Disconnect"));
    disconnectButton->setEnabled(true);
    disconnectButton->setVisible(false);


    idHBox->addWidget(idLabel);
    idHBox->addWidget(idLineEdit);
    idHBox->addWidget(connectButton);
    idHBox->addWidget(disconnectButton);
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

    connect(disconnectButton, &QPushButton::released, this, &GroupSsh::onDisconnect);

    connect(this, &GroupSsh::nfyConnected, this, &GroupSsh::swapConnectButtons);

//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){idLineEdit->setText(str);idLineEdit->setCursorPosition(str.length());});
//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){qDebug() << str;});
    QShortcut* sc = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
    sc->setContext(Qt::ApplicationShortcut);
    connect(sc, &QShortcut::activated, this, &GroupSsh::toggleEnableAdv );

}

void GroupSsh::onDisconnect(){
    qDebug() << "Disconnecting...";
    ssh.disconnect();
    nfyConnected(false);
}

void GroupSsh::onConnect(){
    std::string rpMac = idLineEdit->text().toLower().toStdString();
    qDebug() << QString::fromStdString(rpMac);

    qDebug() << "Connecting...";

    R status = connectToRP(rpMac);

    switch(status){
    case R::ok:
        qDebug() << "ok";
        nfyConnected(true);
        break;
    case R::connection:
        qDebug() << "con";
        //noConnection();
        break;
    case R::other:
        qDebug() << "other";
        //sthWrong();
        break;
    }
}

GroupSsh::R GroupSsh::connectToRP(std::string rpMac){
    std::string host, user;
#ifndef COMD
    host = "rp-" + rpMac + ".local";
    user = "root";
#else
    host = "localhost";
    user = USERD;
#endif
    ssh.setUser(user);
    ssh.setHost(host);
    if(ssh.getStatus() != Ssh::Status::disconnected){
        qDebug() << "Last session wasn't closed."; return R::other;
    }
    if(ssh.connect() != Ssh::R::ok){
        qDebug() << "No connection."; return R::connection;
    }
    ssh.verify();
    if(ssh.getStatus() == Ssh::Status::unknownserv){
        qDebug() << "Accepting:" << QString::fromStdString(ssh.getHash());
        ssh.accept();
    }
    if(ssh.getStatus() != Ssh::Status::verified){
        qDebug() << "Verification error."; ssh.disconnect(); return R::other;
    }
    std::string pass;

#ifndef COMD
    pass = "root";
#else
    pass = QInputDialog::getText(this, tr("Authentication") ,tr("Password"), QLineEdit::Password).toStdString();
#endif

    if(ssh.auth(pass) != Ssh::R::ok){
        qDebug() << "Authentication error."; ssh.disconnect(); return R::other;
    }
    qDebug() << "Connection established.";
    if(ssh.setupSftp() != Ssh::R::ok){
        qDebug() << "Sftp initialization error."; ssh.disconnect(); return R::other;
    }
    return R::ok;
}

void GroupSsh::onLoad(BitstreamSpecs bitSpecs){
    R status = loadBitstream(bitSpecs);
    switch(status){
    case R::ok:
        qDebug() << "ok";
        nfyBitstreamLoaded(bitSpecs.getSpecs());
        break;
    case R::connection:
        qDebug() << "con";
        //lostConnection();
        break;
    case R::other:
        qDebug() << "other";
        //sthWrong();
        break;
    }
}

GroupSsh::R GroupSsh::loadBitstream(BitstreamSpecs bitSpecs){
    qDebug() << bitSpecs.getFilePath();
    return R::ok;
}

void GroupSsh::loadSrcKernel(std::vector<double> crrSrcKer){
    qDebug() << "Loading src kernel.";
}

void GroupSsh::loadKernel(std::vector<double> crrKer){
    qDebug() << "Loading kernel.";
}

void GroupSsh::toggleEnableAdv(){
    enableAdvState = !enableAdvState;
    if(advButton->isChecked())
        advButton->animateClick(0);
    advButton->setEnabled(enableAdvState);
}

void GroupSsh::swapConnectButtons(bool connected){
    if(connected){
        connectButton->setVisible(false);
        disconnectButton->setVisible(true);
    }
    else{
        disconnectButton->setVisible(false);
        connectButton->setVisible(true);
    }
}
