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
#include <QSettings>
#include <QStyleFactory>
#include <QScreen>
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

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":data/icon.png"));
#ifdef _WIN32
    //     Font not looking good with scaling on high dpi
    //     TODO: test with Full HD monitor
    QFont appFont = QApplication::font();
    if(appFont.pointSizeF() > 0.)
        appFont.setPointSize(appFont.pointSize()+1);
    QApplication::setFont(appFont);
#endif

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

#ifndef IS_DEV
    logfile.open("log.txt", std::ofstream::out);
    if (logfile)
        qInstallMessageHandler(logToFile);
#endif

    qInfo() << "libssh version " << ssh_version(0);

    if(cdir.exists("update")){
        QDir updir(cdir);
        if(updir.cd("update")){
            qInfo() << "Cleaning update files.";
            updir.removeRecursively();
        }
    }


#ifdef _WIN32
    QPalette palette;
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
    QSettings dwmSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM",QSettings::NativeFormat);
    unsigned int accent = dwmSettings.value("AccentColor", 0).value<unsigned int>();
    if(accent != 0){
        int accentR = (accent & 0xffu);int accentG = (accent & 0xff00u) >> 8;int accentB = (accent & 0xff0000u) >> 16;
        QColor accentColor =  QColor(accentR, accentG, accentB);
        int accH, accS, accL; accentColor.getHsl(&accH,&accS,&accL);
        //int acclight = accentColor.lightness();
        //if(acclight<150) accentColor = accentColor.lighter((200-acclight+100)*150/225);//magic equation
//        QColor highlightColor; highlightColor.setHsl(accH,accS,220);
//        palette.setColor(QPalette::Highlight,highlightColor);
        //For some reason setting Highlight or highlight text color on palette hides ugly frame on combo boxes,
        //though doesn't affect its color in any way.
        palette.setColor(QPalette::Highlight, QColor(38, 139, 210));

        //int backLight = palette.color(QPalette::Window).lightness();
        QColor backColor; backColor.setRgb(253, 246, 227);
        palette.setColor(QPalette::Window, backColor);
        QColor forgrColor; forgrColor.setRgb(0, 43, 54);
        palette.setColor(QPalette::WindowText, forgrColor);
        palette.setColor(QPalette::Text, forgrColor);
        palette.setColor(QPalette::ButtonText, forgrColor);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText, QColor(88, 110, 117));
    }

    qApp->setPalette(palette);
    if(settings.value("AppsUseLightTheme",1)==0){
        //if dark theme
    }

#endif //_WIN32
    Window window;
    int scrN = app.desktop()->screenNumber(&window);
    if(scrN >= 0){
        QScreen *screen = QGuiApplication::screens().at(scrN);
        QRect wRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,window.size(), screen->geometry());
        int h = screen->geometry().height();
        int delta = h/20;
        int winTop = wRect.y();
        if(delta > winTop)
            delta = winTop;
        qDebug() << wRect;
        wRect.adjust(0, -delta, 0, -delta);
        window.setGeometry(wRect);
    }
    window.show();
    return app.exec();
}
