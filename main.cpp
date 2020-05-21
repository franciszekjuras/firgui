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
#include <QFontDatabase>
#include "window.h"
#include <libssh/libssh.h>
#include <fstream>
//#include <material/lib/qtmaterialstyle.h>
//#include <material/lib/qtmaterialtheme.h>
#include "xcolor.h"

static std::ofstream logfile;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    Q_UNUSED(context)
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
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName(SETTINGSKEY);
    QCoreApplication::setApplicationName(SETTINGSKEY);

    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":data/icon.png"));

    QFontDatabase::addApplicationFont(":fonts/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":fonts/Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(":fonts/Roboto-Light.ttf");
    QFontDatabase fontdb;
    QApplication::setFont(QFont("Roboto",12));

//    QtMaterialTheme* theme = new QtMaterialTheme();
//    theme->setColor("primary1", Material::cyan300);
//    theme->setColor("primary2", Material::cyan700);
//    theme->setColor("primary3", Material::grey400);
//    theme->setColor("accent1", Material::pinkA200);
//    theme->setColor("accent2", Material::grey900);
//    theme->setColor("accent3", Material::grey500);
//    theme->setColor("text", Material::grey50);
//    theme->setColor("alternateText", Material::grey900);
//    theme->setColor("canvas", Material::grey900);
//    theme->setColor("surface", Material::grey800);
//    theme->setColor("flatElevation", Material::white, 0.1);
//    theme->setColor("border", Material::grey700);
//    theme->setColor("thumb", Material::grey300);
//    theme->setColor("raisedElevation", Material::white, 0.13);
//    theme->setColor("surfaceOverlay", Material::white, 0.08);
//    theme->setColor("primaryOverlay", Material::white, 0.2);
//    theme->setColor("disabled", Material::grey600);
//    theme->setColor("disabled2", Material::grey700);
//    theme->setColor("disabled3", Material::grey800);

//    QtMaterialStyle::instance().setTheme(theme);

    QDir startdir;

#ifndef IS_DEV
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

/*
#ifdef _WIN32
    QPalette palette;
    QPalette dialPal;

    //checking dark theme
    QSettings appSet;
    bool isDarkTheme = false;
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
    bool sysDarkTheme = (settings.value("AppsUseLightTheme",1)==0);
    bool lastSysDarkTheme = appSet.value("view/lastSysDarkTheme",sysDarkTheme).toBool();
    if(!appSet.contains("view/darkTheme") || (lastSysDarkTheme != sysDarkTheme)){
        isDarkTheme = sysDarkTheme;
        appSet.setValue("view/darkTheme",sysDarkTheme);
    }else {
        isDarkTheme = appSet.value("view/darkTheme").toBool();
    }
    appSet.setValue("view/lastSysDarkTheme",sysDarkTheme);

    //checking accent color
    QSettings dwmSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM",QSettings::NativeFormat);
    unsigned int accent = dwmSettings.value("AccentColor", 0).value<unsigned int>(); int accentR = (accent & 0xffu);int accentG = (accent & 0xff00u) >> 8;int accentB = (accent & 0xff0000u) >> 16;
    QColor accentColor =  QColor(accentR, accentG, accentB);

    QColor solarAccent = XColor::getSolarizedAccent(accentColor);

    //For some reason setting Highlight or highlight text color on palette hides ugly frame on combo boxes,
    //though doesn't affect its color in any way.
    //highlight (accent) - same for both themes
    palette.setColor(QPalette::Highlight, solarAccent); //this should depend on accent color
    palette.setColor(QPalette::Inactive, QPalette::Highlight, solarAccent.lighter(130));

    if(isDarkTheme){ //dark theme
        //background
        palette.setColor(QPalette::Window, XColor::base03);
        //foreground
        palette.setColor(QPalette::WindowText, XColor::base2);
        palette.setColor(QPalette::Text, XColor::base02);
        palette.setColor(QPalette::ButtonText, XColor::base02);

        //extra foreground
        //palette.setColor(QPalette::Inactive, QPalette::WindowText, XColor::base1);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText, XColor::base01);
    }
    else{ //light theme

        //background
        palette.setColor(QPalette::Window, XColor::base3);
        //foreground
        palette.setColor(QPalette::WindowText, XColor::base02);
        palette.setColor(QPalette::Text, XColor::base02);
        palette.setColor(QPalette::ButtonText, XColor::base02);

        //extra foreground
        //palette.setColor(QPalette::Inactive, QPalette::WindowText, XColor::base01);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText, XColor::base1);

    } //light theme end

    qApp->setPalette(palette);

    //becouse palette doesn't effect dialog background, always set light theme for dialogs
    dialPal.setColor(QPalette::Window, XColor::base3); //currently doesn't work, but just in case...
    dialPal.setColor(QPalette::WindowText, XColor::base02);
    qApp->setPalette(dialPal, "QDialog");
    qApp->setPalette(palette, "TitleBar");

    Window window(isDarkTheme);
    //window.defaultFontSize = defaultPointSize;
#else //LINUX
    Window window;
#endif
*/
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
        wRect.adjust(0, -delta, 0, -delta);
        window.setGeometry(wRect);
    }
    window.show();
    return app.exec();
}
