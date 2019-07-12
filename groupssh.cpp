#include <QtWidgets>
#include <string>
#include <fstream>
#include <QString>
#include <QDebug>
#include <QShortcut>
#include <QInputDialog>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>
#include <QFileInfo>
#include <QMessageBox>
#include "groupssh.h"
#include "switch.h"


GroupSsh::GroupSsh(QWidget *parent) :QGroupBox(tr("Connection"),parent)
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

    QPushButton* testButton = new QPushButton(tr("Test"));

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
#ifdef COMD
    idHBox->addWidget(testButton);
#endif
    idHBox->addWidget(waitSpin);
    idHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));

    /*>>>Widget Layout<<<*/

    vBox->addLayout(idHBox);
    this->setLayout(vBox);

    //---> Functionality <---//

    connect(idLineEdit, &IMLineEdit::returnPressed,[=](){if(connectButton->isVisible()) connectButton->click();});
    connect(connectButton, &QPushButton::released, this, &GroupSsh::onConnect);
    connect(idLineEdit, &QLineEdit::textChanged, [=](const QString& str){connectButton->setEnabled((str.length() == 6));});

    connect(disconnectButton, &QPushButton::released, this, &GroupSsh::onDisconnect);

    connect(this, &GroupSsh::nfyConnected, this, &GroupSsh::swapConnectButtons);

    connect(testButton, &QPushButton::released, this, &GroupSsh::test);

    //connection chain:
    connect(&connectWatch, &QFutureWatcher<R>::finished, this, &GroupSsh::connectToRPFinished);
    connect(&authWatch, &QFutureWatcher<R>::finished, this, &GroupSsh::authenticateRPFinished);

    fcUploaded = false;
}

void GroupSsh::test(){
    qDebug() << "Test";
    ssh.execCommand("echo test >>text.txt");
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
    if(ssh.connect(SSH_TIMEOUT) != Ssh::R::ok){
        qDebug() << "No connection."; return R::connection;
    }
    if(ssh.verify() != Ssh::R::ok){
        ssh.addKnownHost();
        ssh.verify();
    }
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
        QMessageBox::critical(this, tr("Fir Controller - Connection error"), tr("Unexpected error occured while connecting."));
        return;
    }
    if(status == R::connection){
        qDebug() << "Coudn't connect to Red Pitaya.";        
        QMessageBox::warning(this, tr("Fir Controller - Connection error"), tr("Connection with Red Pitaya could not be established. Check if:\n- Red Pitaya is powered on,\n- all cables are attached,\n- proper ID was entered."));
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
        qDebug() << "Authentication failed";
        QMessageBox::critical(this, tr("Fir Controller - Authentication error"), tr("Authentication failed. Maybe someone changed root password on RedPitaya?. If problem persists try flashing fresh system on RedPitaya SD card."));
        break;
    case R::other:
        qDebug() << "Unexpected error";
        QMessageBox::critical(this, tr("Fir Controller - Authentication error"), tr("Unexpected error occured during authentication."));
        //sthWrong();
        break;
    }

    waitSpin->stop();
    connectButton->setEnabled(true);
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
        default:
            return R::other;
        }
    }
    fcMajVer = mV;
    fcSubVer = subDec;
    qDebug() << "firctrl successfully sent.";
    return R::ok;

}

void GroupSsh::onLoad(BitstreamSpecs bitSpecs){
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    R status = loadBitstream(bitSpecs);

    QApplication::restoreOverrideCursor();

    switch(status){
    case R::ok:
        qDebug() << "ok";
        nfyBitstreamLoaded(bitSpecs.getSpecs());
        break;
    case R::connection:
        qDebug() << "Connection lost.";
        onDisconnect();
        QMessageBox::critical(this, tr("Fir Controller - Connection lost."), tr("Connection lost. Try connecting again."));
        break;
    default:
        qDebug() << "Unexpected error occured during bitstream loading.";
        QMessageBox::critical(this, tr("Fir Controller - Unexpected error."), tr("Unexpected error occured when loading bitstream."));
        break;
    }
}

GroupSsh::R GroupSsh::loadBitstream(BitstreamSpecs bitSpecs){

    std::string bitPath = bitSpecs.getFilePath().toStdString();

#ifndef COMD

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
        default:
            return R::other;
        }
    }

#endif //COMD
    Ssh::R stat = ssh.execCommand("echo"); //check connection
    switch(stat){
    case Ssh::R::ok:
        break;
    case Ssh::R::connection:
        return R::connection;
    default:
        return R::other;
    }
    qDebug() << "Uploading bitstream:" << QString::fromStdString(bitPath);
    stat = ssh.sendFileToFile(bitPath,"/tmp/bitstream.bin");
    if(stat == Ssh::R::ok){
        //TODO: check sha1sum of uploaded bitstream
#ifdef COMD
        stat = ssh.execCommand(".local/bin/lconf /tmp/bitstream.bin");
#else
        stat = ssh.execCommand("lconf /tmp/bitstream.bin");
#endif

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
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    Ssh::R stat = ssh.execCommand("echo"); //check connection
    if(stat == Ssh::R::ok){
        qDebug() << "Uploading src kernel...";
        stat = ssh.sendMemToFile((void*)crrSrcKer.data(), crrSrcKer.size()*sizeof(double), "/tmp/srcker.dat");
        //local copy
        std::ofstream ofp("srcker.dat", std::ios::out | std::ios::binary);
        ofp.write(reinterpret_cast<const char*>(crrSrcKer.data()), crrSrcKer.size() * sizeof(crrSrcKer[0]));ofp.close();
    }
    QApplication::restoreOverrideCursor();

    if(stat == Ssh::R::ok){
        qDebug() << "ok";
        return;
    }
    if(stat == Ssh::R::connection){
        qDebug() << "Connection lost.";
        QMessageBox::critical(this, tr("Fir Controller - Connection lost."), tr("Connection lost. Try connecting again."));
        onDisconnect();
        return;
    }
    //other:
    QMessageBox::critical(this, tr("Fir Controller - Unexpected error."), tr("Unexpected error occured when loading SRC kernel."));
    qDebug() << "Unexpected error occured during src kernel loading.";
}

void GroupSsh::loadKernel(std::vector<double> crrKer){
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    Ssh::R stat = ssh.execCommand("echo"); //check connection
    if(stat == Ssh::R::status){
        QApplication::restoreOverrideCursor();
        return; //this covers situation when connection brakes during loadSrcKernel
    }           //but groupspecs doesn't know that and calls loadKernel anyway
    if(stat == Ssh::R::ok){
        qDebug() << "Uploading kernel...";
        Ssh::R stat = ssh.sendMemToFile((void*)crrKer.data(), crrKer.size()*sizeof(double), "/tmp/firker.dat");
        //local copy
        std::ofstream ofp("firker.dat", std::ios::out | std::ios::binary);
        ofp.write(reinterpret_cast<const char*>(crrKer.data()), crrKer.size() * sizeof(crrKer[0]));ofp.close();
    }
    if(stat == Ssh::R::ok){
        qDebug() << "ok";
#ifdef COMD
        stat = ssh.execCommand(".local/bin/firctrl --load");
#else
        stat = ssh.execCommand("firctrl --load");
#endif
        if(stat == Ssh::R::ok){
            qDebug() << "o:" << QString::fromStdString(ssh.getSshOut());
            qDebug() << "e:" << QString::fromStdString(ssh.getSshErr());
#ifdef COMD
            stat = ssh.execCommand(".local/bin/firctrl --enable");
#else
            stat = ssh.execCommand("firctrl --enable");
#endif
            stat = ssh.execCommand("firctrl --enable");
        }
    }
    QApplication::restoreOverrideCursor();

    if(stat == Ssh::R::ok){
        return;
    }
    if(stat == Ssh::R::connection){
        qDebug() << "Connection lost.";
        QMessageBox::critical(this, tr("Fir Controller - Connection lost."), tr("Connection lost. Try connecting again."));
        onDisconnect();
        return;
    }
    QMessageBox::critical(this, tr("Fir Controller - Unexpected error."), tr("Unexpected error occured when loading filter kernel."));
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
