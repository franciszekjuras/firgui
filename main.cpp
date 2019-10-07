#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QSystemSemaphore>
#include <QProcess>
#include "window.h"
#include <libssh/libssh.h>

#include <fstream>

std::ofstream logfile;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    QString datetime = QDateTime::currentDateTime().toString("hh:mm:ss");
    switch (type) {
    case QtDebugMsg:
        logfile << datetime.toStdString() << " [Debug] " << msg.toStdString() << "\n";
        break;
    case QtCriticalMsg:
        logfile << datetime.toStdString() << " [Critical] " << msg.toStdString() << "\n";
        break;
    case QtWarningMsg:
        logfile << datetime.toStdString()<< " [Warning] " << msg.toStdString() << "\n";
        break;
    case QtInfoMsg:
        logfile << datetime.toStdString() <<  " [Info] " << msg.toStdString() << "\n";
        break;
    case QtFatalMsg:
        logfile << datetime.toStdString() <<  " [Fatal] " << msg.toStdString() << "\n";
        logfile.flush();
        abort();
  }
  logfile.flush();
}


int main(int argc, char *argv[])
{


    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":data/icon.png"));

    QDir startdir;

#ifdef WORKDIR_STARTUP_CHECK
#ifdef _WIN32
    if(!startdir.exists("FIR Controller.exe")){
#else
    if(!startdir.exists("FIR Controller")){
#endif
        QDir::setCurrent(QApplication::applicationDirPath());
    }
#endif
    QDir cdir;
    if(cdir.exists("update/_update")){
        if(cdir.remove("update/_update")){
#ifdef _WIN32
            if(cdir.exists("update/updater/updater.exe")){
#else
            if(cdir.exists("update/updater/updater")){
#endif
                QSystemSemaphore sem( "firgui_update", 1, QSystemSemaphore::Create );
                sem.acquire();
#ifdef _WIN32
                if(QProcess::startDetached("update/updater/updater.exe")){
#else
                if(QProcess::startDetached("update/updater/updater")){
#endif
                    return 0;
                }
                else{QMessageBox::critical(nullptr, WINDOW_TITLE, "Update could not be applied.");}
            }
        }
        else{QMessageBox::critical(nullptr, WINDOW_TITLE, "Update could not be applied.");}
    }

    logfile.open("log.txt", std::ofstream::out);
    if (logfile)
        qInstallMessageHandler(logToFile);

    qInfo() << "libssh version " << ssh_version(0);

    if(cdir.exists("update")){
        QDir updir(cdir);
        if(updir.cd("update")){
            qInfo() << "Cleaning update files.";
            updir.removeRecursively();
        }
    }


    Window window;
    QRect wRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,window.size(),app.desktop()->availableGeometry());
    wRect.adjust(0, -50, 0, -50);
    window.setGeometry(wRect);
    window.show();
    return app.exec();
}
