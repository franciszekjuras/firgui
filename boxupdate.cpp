#include <QtWidgets>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include "boxupdate.h"

BoxUpdate::BoxUpdate(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout* updateHBox = new QHBoxLayout;
    this->setLayout(updateHBox);
    //updateHBox --v

        updateLabel = new QLabel;
        updateHBox->addWidget(updateLabel);

        updateHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));

        downloadButton = new QPushButton(tr("Download"));
        updateHBox->addWidget(downloadButton);

        updateFailedMoreButton = new QPushButton(tr("OK"));
        updateHBox->addWidget(updateFailedMoreButton);


        downloadLabel = new QLabel();
        downloadLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        updateHBox->addWidget(downloadLabel);

        downloadProgress = new QProgressBar();
        downloadProgress->setTextVisible(false);
        updateHBox->addWidget(downloadProgress);

        this->setFixedHeight(this->sizeHint().height());

        this->hide();
        updateLabel->hide();
        downloadButton->hide();
        updateFailedMoreButton->hide();
        downloadLabel->hide();
        downloadProgress->hide();

//function:
#ifdef IS_PRERELEASE
    updateToPreReleases = true;
#else
    updateToPreReleases = false;
#endif

    updater = new CAutoUpdaterGithub(GITHUBREPO, QString(VERSIONSTRING) + VERSIONTYPE);
    connect(updater, &CAutoUpdaterGithub::updateAvailable, this, &BoxUpdate::onUpdateAvailable);
    connect(updater, &CAutoUpdaterGithub::downloadProgress, this, &BoxUpdate::onDownloadProgress);
    connect(updater, &CAutoUpdaterGithub::updateDownloadFinished, this, &BoxUpdate::onUpdateDownloadFinished);
    connect(updater, &CAutoUpdaterGithub::updateError, this, &BoxUpdate::onUpdateError);
    connect(downloadButton, &QPushButton::clicked, this, &BoxUpdate::onDownloadUpdate);
    connect(updateFailedMoreButton, &QPushButton::clicked, this, &BoxUpdate::onUpdateFailedMore);

    QDir cdir; bool lastUpdateFailed = false;
    if(cdir.exists("updatestatus")){
        QFile uplog("updatestatus"); uplog.open(QIODevice::ReadOnly);
        QString uplogText = uplog.readAll();
        uplog.close();
        if(uplogText.left(2).compare("ok", Qt::CaseInsensitive)!=0)
            lastUpdateFailed = true;
    }
    if(lastUpdateFailed){
        this->show();
        updateLabel->setText("Recent update failed.");
        updateLabel->show();
        updateFailedMoreButton->show();
    }
    else
        QTimer::singleShot(5000, updater, &CAutoUpdaterGithub::checkForUpdates);
}

bool BoxUpdate::prepareEnvironment(){
    //Level of checking might be a small overkill,
    //but it's always better to make sure that update won't mess up filesystem
    QDir cdir;
    if(cdir.exists("update")){
        QDir updir(cdir);
        if(updir.cd("update") && updir.removeRecursively()){
            qInfo() << "Cleaning update files.";
        }
        else {qCritical() << "Cannot remove update folder to perform update."; return false;}
    }
    QDir updir(cdir);
    if(!cdir.mkdir("update") || !updir.cd("update"))
        {qCritical() << "Cannot access update folder to perform update."; return false;}
    QDir uptrdir(updir);
    if(!updir.mkdir("updater") || !uptrdir.cd("updater"))
        {qCritical() << "Cannot create updater folder to perform update."; return false;}
#ifdef _WIN32
    if(!uptrdir.mkdir("platforms") || !uptrdir.mkdir("styles"))
        {qCritical() << "Cannot create platforms and styles folders to perform update."; return false;}
    if(!QFile::copy("updater.exe", "update/updater/updater.exe"))
        {qCritical() << "Cannot copy updater executable."; return false;}
    if(!QFile::copy("Qt5Core.dll", "update/updater/Qt5Core.dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    if(!QFile::copy("Qt5Gui.dll", "update/updater/Qt5Gui.dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    if(!QFile::copy("Qt5Widgets.dll", "update/updater/Qt5Widgets.dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    if(!QFile::copy("platforms/qwindows.dll", "update/updater/platforms/qwindows.dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    if(!QFile::copy("styles/qwindowsvistastyle.dll", "update/updater/styles/qwindowsvistastyle.dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    //Visual C++ Runtime
    if(!QFile::copy(QString("msvcp") + MSVISUALDLLSUFIX + ".dll",
                    QString("update/updater/msvcp") + MSVISUALDLLSUFIX + ".dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    if(!QFile::copy(QString("vcomp") + MSVISUALDLLSUFIX + ".dll",
                    QString("update/updater/vcomp") + MSVISUALDLLSUFIX + ".dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}
    if(!QFile::copy(QString("vcruntime") + MSVISUALDLLSUFIX + ".dll",
                    QString("update/updater/vcruntime") + MSVISUALDLLSUFIX + ".dll"))
        {qCritical() << "Cannot copy updater libraries."; return false;}

    //7-zip executable
    if(!cdir.exists("7za.exe"))
        {qCritical() << "Cannot find 7zip executable."; return false;}
#else
    if(!QFile::copy("updater", "update/updater/updater"))
        {qCritical() << "Cannot copy updater executable."; return false;}
    if(!cdir.exists("p7zip"))
        {qCritical() << "Cannot find 7zip executable."; return false;}
#endif
    return true;
}

void BoxUpdate::onDownloadUpdate(){
    downloadButton->hide();
    //prepare environment
    QDir cdir;
#ifdef _WIN32
    if(!cdir.exists("FIR Controller.exe")){
#else
    if(!cdir.exists("FIR Controller")){
#endif
        qCritical() << "Cannot find application directory to perform update.";
        updateLabel->setText("Update failed.");return;
    }
    if(!prepareEnvironment()){
        qCritical() << "Could not prepare environment for update.";
        updateLabel->setText("Update failed.");
        return;
    }
    qInfo() << "Update environment succesfully prepared.";
    updateLabel->setText("Downloading");

    //download
    updater->downloadUpdate(proposedUpdate.versionUpdateUrl, "update/");
}


void BoxUpdate::onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog){
    qInfo() << "Available updates:";
    for(auto it : changelog){
        qInfo() << it.versionString;
    }
    for(auto it : changelog){
        if(updateToPreReleases == it.versionString.contains("beta")){
            this->show();
            proposedUpdate = it;
            updateLabel->setText(tr("Update available: v") + it.versionString);
            updateLabel->show();
            downloadButton->show();
            break;
        }
    }
}

void BoxUpdate::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal){
    if(downloadLabel->isHidden()){
        downloadLabel->setText(QString::number(static_cast<double>(bytesTotal)/1048576.,'f',1) + "/" +
                               QString::number(static_cast<double>(bytesTotal)/1048576.,'f',1) + "MB");
        downloadLabel->setFixedWidth(downloadLabel->sizeHint().width() + 5);

        downloadProgress->setMaximum(static_cast<int>(bytesTotal));

        downloadLabel->show();
        downloadProgress->show();
    }
    qInfo() << bytesReceived << "/" << bytesTotal;
    downloadLabel->setText(QString::number(static_cast<double>(bytesReceived)/1048576.,'f',1) + "/" +
                           QString::number(static_cast<double>(bytesTotal)/1048576.,'f',1) + "MB");
    downloadProgress->setValue(static_cast<int>(bytesReceived));
}

void BoxUpdate::onUpdateDownloadFinished(){
    qInfo() << "Download Finished";
    downloadProgress->hide();
    downloadLabel->hide();
    updateLabel->setText("Unpacking archive...");
    QProcess* unzipProc = new QProcess(this);
    unzipProc->setWorkingDirectory("update");
    connect(unzipProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BoxUpdate::onArchiveUnpacked);
    connect(unzipProc, &QProcess::errorOccurred, [=](QProcess::ProcessError error){
        qCritical() << "Unpacking process error:" << error;
        this->updateLabel->setText("Update failed.");
    });
    unzipProc->start("7za.exe x appupdate" + UPDATE_FILE_EXTENSION);
}

void BoxUpdate::onArchiveUnpacked(int exitCode, QProcess::ExitStatus exitStatus){
    if(exitCode != 0){
        qCritical() << "Unpacking process returned" << exitCode;
        updateLabel->setText("Update failed."); return;
    }
    QDir cdir;
    cdir.cd("update");
    cdir.remove("appupdate" + UPDATE_FILE_EXTENSION);
    auto upDirContent = cdir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    if(upDirContent.removeOne("updater") && (upDirContent.length() == 1)){
        if(!cdir.rename(upDirContent.first(),"app")){
            qCritical() << "Could not change" << upDirContent.first() << "name to app";
            updateLabel->setText("Update failed."); return;
        }
    } else{
        qCritical() << "Unexpected content of update folder";
        updateLabel->setText("Update failed."); return;
    }

    QFile file("update/_update");
    if(!file.open(QIODevice::WriteOnly)){
        qCritical() << "Could not create update/_update file.";
        updateLabel->setText("Update failed."); return;
    }
    file.close();
    updateLabel->setText("Restart to apply update.");
}

void BoxUpdate::onUpdateError(QString errorMessage){
    qCritical() << "Update error:" << errorMessage;
    downloadProgress->hide();
    downloadLabel->hide();
    updateLabel->setText("Update failed.");
}

void BoxUpdate::onUpdateFailedMore(){
    QFile::remove("updatestatus");
    updateLabel->hide();
    updateFailedMoreButton->hide();
    QTimer::singleShot(5000, updater, &CAutoUpdaterGithub::checkForUpdates);
}
