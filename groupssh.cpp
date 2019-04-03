#include <QtWidgets>
#include <string>
#include <QString>
#include <QDebug>
#include <QShortcut>
#include <QInputDialog>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>
#include <QFileInfo>
#include "groupssh.h"
#include "switch.h"


GroupSsh::GroupSsh(QWidget *parent) :QGroupBox(tr("SSH options"),parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    /*>>>Layouts<<<*/

    QVBoxLayout* vBox = new QVBoxLayout;
    QHBoxLayout* idHBox = new QHBoxLayout;
//    QGridLayout* commandGrid = new QGridLayout;

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

    waitSpin = new WaitingSpinnerWidget(0, false, false);
    waitSpin->setRoundness(70.0);
    waitSpin->setMinimumTrailOpacity(50.0);
    waitSpin->setTrailFadePercentage(70.0);
    waitSpin->setNumberOfLines(10);
    waitSpin->setLineLength(6);
    waitSpin->setLineWidth(3);
    waitSpin->setInnerRadius(5);
    waitSpin->setRevolutionsPerSecond(2);
    waitSpin->setColor(QColor(0, 150, 136));


    idHBox->addWidget(idLabel);
    idHBox->addWidget(idLineEdit);
    idHBox->addWidget(connectButton);
    idHBox->addWidget(disconnectButton);
    idHBox->addWidget(waitSpin);
    idHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));

    //QSpacerItem* scSpacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred);

    /*>>>Advanced Options<<<*/

    //QSpacerItem* advSpacer = new QSpacerItem(0, 10);
//    advButton = new Switch(tr("Advanced Options"));
//    enableAdvState = true; toggleEnableAdv();


    //advButton->setCheckable(true);

    /*>>>Command Grid<<<*/

//    QLineEdit* commandLine = new QLineEdit;
//    QPushButton* commandButton = new QPushButton(tr("Send"));
//    QFontMetrics fm = commandButton->fontMetrics();
//    commandButton->setMaximumWidth(fm.width(tr("Send"))+20);
//    QTextBrowser* commandBrowser = new QTextBrowser;
//    commandBrowser->setFixedHeight(70);

//    commandGrid->addWidget(commandLine, 0, 0);
//    commandGrid->addWidget(commandButton, 0, 1);
//    commandGrid->addWidget(commandBrowser, 1, 0, 1, 2);

    //set visibility toogle
//    commandBrowser->setVisible(false);commandLine->setVisible(false);commandButton->setVisible(false);
//    connect(advButton, SIGNAL(toggled(bool)), commandBrowser, SLOT(setVisible(bool)));
//    connect(advButton, SIGNAL(toggled(bool)), commandLine, SLOT(setVisible(bool)));
//    connect(advButton, SIGNAL(toggled(bool)), commandButton, SLOT(setVisible(bool)));



    /*>>>Widget Layout<<<*/

    vBox->addLayout(idHBox);
    //vBox->addItem(advSpacer);
//    vBox->addWidget(advButton);
//    vBox->addLayout(commandGrid);
    //vBox->addWidget(responseTextBox);
    this->setLayout(vBox);

    //---> Functionality <---//

    connect(connectButton, &QPushButton::released, this, &GroupSsh::onConnect);
    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){connectButton->setEnabled((str.length() == 6));});

    connect(disconnectButton, &QPushButton::released, this, &GroupSsh::onDisconnect);

    connect(this, &GroupSsh::nfyConnected, this, &GroupSsh::swapConnectButtons);


    //connection chain:
    connect(&connectWatch, &QFutureWatcher<R>::finished, this, &GroupSsh::connectToRPFinished);
    connect(&authWatch, &QFutureWatcher<R>::finished, this, &GroupSsh::authenticateRPFinished);

    fcUploaded = false;


//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){idLineEdit->setText(str);idLineEdit->setCursorPosition(str.length());});
//    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){qDebug() << str;});
//    QShortcut* sc = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
//    sc->setContext(Qt::ApplicationShortcut);
//    connect(sc, &QShortcut::activated, this, &GroupSsh::toggleEnableAdv );

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

    fcUploaded = false;
    connectButton->setDisabled(true);
    waitSpin->start();
    QFuture<R> fut = QtConcurrent::run([=](){return this->connectToRP(rpMac);});
    connectWatch.setFuture(fut);
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
    return R::ok;
}

void GroupSsh::connectToRPFinished(){
    R status = connectWatch.future().result();

    if(status == R::ok){
        std::string pass;
        #ifndef COMD
        pass = "root";
        #else
        pass = QInputDialog::getText(this, tr("Authentication") ,tr("Password"), QLineEdit::Password).toStdString();
        #endif
        QFuture<R> fut = QtConcurrent::run([=](){return this->authenticateRP(pass);});
        authWatch.setFuture(fut);
        return;
    }

    waitSpin->stop();    
    connectButton->setEnabled(true);

    if(status == R::other){
        qDebug() << "Unexpected error occured while connecting.";
        return;
    }
    if(status == R::connection){
        qDebug() << "Coudn't connect to Red Pitaya.";
        return;
    }
}

GroupSsh::R GroupSsh::authenticateRP(std::string pass){
    if(ssh.auth(pass) != Ssh::R::ok){
        qDebug() << "Authentication error."; ssh.disconnect(); return R::auth;
    }
    qDebug() << "Connection established.";
    if(ssh.setupSftp() != Ssh::R::ok){
        qDebug() << "Sftp initialization error."; ssh.disconnect(); return R::other;
    }

    //TODO: prepare environment

    //rw
    //send lconf
    //send firctrl


    return R::ok;
}

void GroupSsh::authenticateRPFinished(){

    R status = authWatch.future().result();

    switch(status){
    case R::ok:
        qDebug() << "ok";
        nfyConnected(true);
        break;
    case R::auth:
        qDebug() << "auth";
        //noConnection();
        break;
    case R::other:
        qDebug() << "other";
        //sthWrong();
        break;
    }

    waitSpin->stop();
    connectButton->setEnabled(true);
}

void GroupSsh::onLoad(BitstreamSpecs bitSpecs){
    R status = loadBitstream(bitSpecs);
    switch(status){
    case R::ok:
        qDebug() << "ok";
        nfyBitstreamLoaded(bitSpecs.getSpecs());
        //TODO: print info (label?)
        break;
    case R::connection:
        qDebug() << "Connection lost.";
        onDisconnect();
        //TODO: print info (label?)
        break;
    case R::other:
        qDebug() << "Unexpected error occured during bitstream loading.";
        //TODO: dialog
        break;
    }
}

GroupSsh::R GroupSsh::uploadFirCtrl(const BitstreamSpecs& bitSpecs){
    int mV = bitSpecs.getMajVersion();
    int sV = bitSpecs.getSubVersion();
    int subDec = sV/10;
    if(fcUploaded){
        if(fcMajVer == mV && fcSubVer == subDec)
            return R::ok;
    }
    QString fcFileName = "firctrl_";
    fcFileName.append(QString::number(mV));
    fcFileName.append("_");
    fcFileName.append(QString::number(subDec));
    fcFileName.append("_*");

    qDebug() << "fcFileName:" << fcFileName;


    QDir rpDir("data/redpitaya");
    qDebug() << "redpitaya dir exists:"<< rpDir.exists();
    QFileInfoList fcFiles = rpDir.entryInfoList(QStringList(fcFileName), QDir::Files, QDir::Name);
    if(fcFiles.isEmpty()){
        qDebug()<< "No matching firctrl found.";
        return R::other;
    }
    QFileInfo fcFile = fcFiles.back();
    qDebug() << fcFile.filePath();

    Ssh::R r = ssh.sendFileToFile(fcFile.filePath().toStdString(),"/usr/local/bin/firctrl");
    if(r != Ssh::R::ok){
        switch (r) {
        case Ssh::R::connection:
            return R::connection;
        case Ssh::R::authentication:
            return R::auth;
        default:
            return R::other;
        }
    }
    fcMajVer = mV;
    fcSubVer = subDec;
    qDebug() << "firctrl successfully sent.";
    return R::ok;

}


GroupSsh::R GroupSsh::loadBitstream(BitstreamSpecs bitSpecs){
    std::string bitPath = bitSpecs.getFilePath().toStdString();

    R upfcstat = uploadFirCtrl(bitSpecs);
    if(upfcstat != R::ok)
        return upfcstat;

    QFileInfo lconfFI("data/redpitaya/lconf");
    if(!lconfFI.exists() || !lconfFI.isFile())
        return R::other;
    Ssh::R lcstat = ssh.sendFileToFile(lconfFI.filePath().toStdString(),"/usr/local/bin/lconf");
    if(lcstat != Ssh::R::ok){
        switch (lcstat) {
        case Ssh::R::connection:
            return R::connection;
        case Ssh::R::authentication:
            return R::auth;
        default:
            return R::other;
        }
    }

    qDebug() << "Uploading bitstream:" << QString::fromStdString(bitPath);
    Ssh::R stat = ssh.sendFileToFile(bitPath,"/tmp/bitstream.bin");
    if(stat == Ssh::R::ok){
        //TODO: check sha1sum of uploaded bitstream
        stat = ssh.execCommand("lconf /tmp/bitstream.bin");
        if(stat == Ssh::R::ok){
            qDebug() << "o:" << QString::fromStdString(ssh.getSshOut());
            qDebug() << "e:" << QString::fromStdString(ssh.getSshErr());
            return R::ok;
        }
    }
    if(stat == Ssh::R::connection)
        return R::connection;
    return R::other;
}

void GroupSsh::loadSrcKernel(std::vector<double> crrSrcKer){
    qDebug() << "Uploading src kernel...";
    Ssh::R stat = ssh.sendMemToFile((void*)crrSrcKer.data(), crrSrcKer.size()*sizeof(double), "/tmp/srcker.dat");
    if(stat == Ssh::R::ok){
        qDebug() << "Done";
        return;
    }
    if(stat == Ssh::R::connection){
        qDebug() << "Connection lost.";
        //TODO: print info (label?)
        onDisconnect();
        return;
    }
    //TODO: dialog
    qDebug() << "Unexpected error occured during src kernel loading.";
}

void GroupSsh::loadKernel(std::vector<double> crrKer){
    qDebug() << "Uploading kernel...";
    Ssh::R stat = ssh.sendMemToFile((void*)crrKer.data(), crrKer.size()*sizeof(double), "/tmp/firker.dat");
    if(stat == Ssh::R::ok){
        qDebug() << "Done";
        stat = ssh.execCommand("firctrl --load");
        if(stat == Ssh::R::ok){
            qDebug() << "o:" << QString::fromStdString(ssh.getSshOut());
            qDebug() << "e:" << QString::fromStdString(ssh.getSshErr());

            stat = ssh.execCommand("firctrl --enable");
            if(stat == Ssh::R::ok){
                return;
            }
        }
    }
    if(stat == Ssh::R::connection){
        qDebug() << "Connection lost.";
        //TODO: print info (label?)
        onDisconnect();
        return;
    }
    //TODO: dialog
    qDebug() << "Unexpected error occured during kernel loading.";
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
