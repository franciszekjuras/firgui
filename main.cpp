#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QDebug>
#include <QFile>
#include <QDateTime>
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
        abort();
  }
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    logfile.open("log.txt", std::ofstream::out);
    qInstallMessageHandler(logToFile);

    app.setWindowIcon(QIcon(":data/icon.png"));

    qDebug() << "libssh: " << ssh_version(0);

    Window window;
    QRect wRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,window.size(),app.desktop()->availableGeometry());
    wRect.adjust(0, -50, 0, -50);
    window.setGeometry(wRect);
    window.show();
    return app.exec();
}
