#include <QtWidgets>

#include "window.h"
#include "groupssh.h"
#include "groupconfig.h"
#include "groupspecs.h"
#include "kerplot.h"
#include "switch.h"
#include "boxupdate.h"
#ifdef _WIN32
#include "delegate.h"
#endif
#include "clicklabel.h"
#include "xcolor.h"
#include "QTextBrowser"

Window::Window(bool darkTheme, QWidget *parent)
    : QMainWindow(parent)
{
    isDarkTheme = darkTheme;
//part:layout

QWidget *centralWidget = new QWidget();
setCentralWidget(centralWidget);
QHBoxLayout* mainHBox = new QHBoxLayout;
centralWidget->setLayout(mainHBox);
//mainHBox --v

    QWidget* lWid = new QWidget;
    mainHBox->addWidget(lWid);
    lWid->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    QVBoxLayout* lVBox = new QVBoxLayout;
    lWid->setLayout(lVBox);
    lVBox->setContentsMargins(0,0,0,0);

        ClickLabel* introClickLabel = new ClickLabel(tr("Get Started"));
        introClickLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lVBox->addWidget(introClickLabel);

        GroupConfig* groupConfig = new GroupConfig;
        lVBox->addWidget(groupConfig);
        groupConfig->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

        GroupSpecs* groupSpecs = new GroupSpecs;
        lVBox->addWidget(groupSpecs);
        groupSpecs->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

        GroupSsh* groupSsh = new GroupSsh;
        lVBox->addWidget(groupSsh);
        groupSsh->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

        lVBox->addItem(new QSpacerItem(0,0,QSizePolicy::Preferred, QSizePolicy::Expanding));

        BoxUpdate* boxUpdate = new BoxUpdate;
        lVBox->addWidget(boxUpdate);

        QWidget* lbotWid = new QWidget;
        lVBox->addWidget(lbotWid);
        QHBoxLayout* lbotHBox = new QHBoxLayout;
        lbotWid->setLayout(lbotHBox);

            Switch* showTooltipSwitch = new Switch(tr("Show Tooltips"));
            //lbotHBox->addWidget(showTooltipSwitch);
            showTooltipSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            showTooltipSwitch->setFocusPolicy(Qt::ClickFocus);


            //lbotHBox->addItem(new QSpacerItem(5,0,QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));


            ClickLabel* decrText = new ClickLabel();
            decrText->setText("<p><small>A<sup>―</sup></small></p>");
            lbotHBox->addWidget(decrText);
            connect(decrText, &ClickLabel::clicked, [=](){
                QFont appFont = QApplication::font();
                if(appFont.pointSize()>7)
                    {appFont.setPointSize(appFont.pointSize()-1);}
                QSettings fontSet; fontSet.setValue("view/fontSize",appFont.pointSize());
                QApplication::setFont(appFont);});

            ClickLabel* resetText = new ClickLabel();
            resetText->setText("<p>A<sup>0</sup></p>");//A<sup>0</sup></p>");
            lbotHBox->addWidget(resetText);
            connect(resetText, &ClickLabel::clicked, [=](){
                QFont appFont = QApplication::font();
                if(defaultFontSize > 0){appFont.setPointSize(defaultFontSize);}
                QSettings fontSet; fontSet.setValue("view/fontSize",appFont.pointSize());
                QApplication::setFont(appFont);});

            ClickLabel* incrText = new ClickLabel();
            incrText->setText("<p><big>A<sup>+</sup></big></p>");
            lbotHBox->addWidget(incrText);
            connect(incrText, &ClickLabel::clicked, [=](){
                QFont appFont = QApplication::font();
                if(appFont.pointSize()>0){
                    appFont.setPointSize(appFont.pointSize()+1);}
                QSettings fontSet; fontSet.setValue("view/fontSize",appFont.pointSize());
                QApplication::setFont(appFont);});

#ifdef _WIN32
            Switch* themeSwitch = new Switch(tr("Dark theme"));
            lbotHBox->addWidget(themeSwitch);
            themeSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            themeSwitch->setFocusPolicy(Qt::ClickFocus);
#endif

            lbotHBox->addItem(new QSpacerItem(15,0,QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));

            lbotHBox->addWidget(new QLabel(QString("v") + VERSIONSTRING + VERSIONTYPE));


    QWidget* rWid = new QWidget;
    mainHBox->addWidget(rWid);
    rWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout* rVBox = new QVBoxLayout;
    rWid->setLayout(rVBox);
    rVBox->setContentsMargins(0,0,0,0);
    //rVBox --v

/*
        QWidget* plPropWid = new QWidget;
        rVBox->addWidget(plPropWid);
        plPropWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QHBoxLayout* plPropHBox = new QHBoxLayout;
        plPropWid->setLayout(plPropHBox);
        plPropHBox->setContentsMargins(0,0,0,0);
        //plPropHBox --v

            QComboBox* plotTypeCombo = new QComboBox;
#ifdef _WIN32
            plotTypeCombo->view()->setItemDelegate(new PopupItemDelegate(plotTypeCombo));
#endif
            plPropHBox->addWidget(plotTypeCombo);
            plotTypeCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QComboBox* plotPerfCombo = new QComboBox;
#ifdef _WIN32
            plotPerfCombo->view()->setItemDelegate(new PopupItemDelegate(plotPerfCombo));
#endif
            plPropHBox->addWidget(plotPerfCombo);
            plotPerfCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            plPropHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

*/

        QWidget* plCtrlWid = new QWidget;
        rVBox->addWidget(plCtrlWid);
        plCtrlWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        QHBoxLayout* plCtrlHBox = new QHBoxLayout;
        plCtrlWid->setLayout(plCtrlHBox);
        plCtrlHBox->setContentsMargins(0,0,0,0);
        //plCtrlHBox --v

            Switch* firTransShowSwitch = new Switch(tr("Filter transmission"),QBrush(QColor(255, 119, 0)));
            plCtrlHBox->addWidget(firTransShowSwitch);
            firTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            firTransShowSwitch->setFocusPolicy(Qt::ClickFocus);

            Switch* srcTransShowSwitch = new Switch(tr("Rate conversion trans."),QBrush(QColor(132,186,91)));
            plCtrlHBox->addWidget(srcTransShowSwitch);
            srcTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            srcTransShowSwitch->setFocusPolicy(Qt::ClickFocus);

            Switch* totalTransShowSwitch = new Switch(tr("Total trans."),QBrush(QColor(114,147,203)));
            plCtrlHBox->addWidget(totalTransShowSwitch);
            totalTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            totalTransShowSwitch->setFocusPolicy(Qt::ClickFocus);

            plCtrlHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));


            ClickLabel* helpClickLabel = new ClickLabel("<big>?</big>");
            plCtrlHBox->addWidget(helpClickLabel);


            QComboBox* plotTypeCombo = new QComboBox;
#ifdef _WIN32
            plotTypeCombo->view()->setItemDelegate(new PopupItemDelegate(plotTypeCombo));
#endif
            plCtrlHBox->addWidget(plotTypeCombo);
            plotTypeCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);



    KerPlot* kerPlot = new KerPlot;
    rVBox->addWidget(kerPlot);
    kerPlot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);


    helpBrowser = new QTextBrowser;
    int n10width = this->fontMetrics().horizontalAdvance("nnnnnnnnnn");
    helpBrowser->resize(n10width*7, n10width*7);
    helpBrowser->setWindowTitle(tr(WINDOW_TITLE));
    helpBrowser->setAttribute(Qt::WA_QuitOnClose, false);

//part:funtion

    //:groupSpecs
    connect(groupSpecs, &GroupSpecs::kernelChanged, kerPlot, &KerPlot::setKernel);
    connect(groupSpecs, &GroupSpecs::srcKernelChanged, kerPlot, &KerPlot::setSrcKernel);
    connect(groupSpecs, &GroupSpecs::resetPlot, kerPlot, &KerPlot::resetPlot);
    connect(groupSpecs, &GroupSpecs::reqLoadSrcKernel, groupSsh, &GroupSsh::loadSrcKernel);
    connect(groupSpecs, &GroupSpecs::reqLoadKernel, groupSsh, &GroupSsh::loadKernel);
    connect(groupSpecs, &GroupSpecs::showHelp, [=](){showHelp("sec.specs");});

    //:groupConfig
    connect(groupConfig, &GroupConfig::bitstreamSelected, groupSpecs, &GroupSpecs::bitstreamChanged);
    connect(groupConfig, &GroupConfig::fpgaSamplingFreqChanged, groupSpecs, &GroupSpecs::setfpgaSamplingFreq);
    connect(groupConfig, &GroupConfig::reqLoad, groupSsh, &GroupSsh::onLoad);
    connect(groupConfig, &GroupConfig::showHelp, [=](){showHelp("sec.conf");});
    groupConfig->init();

    //:groupSsh
    connect(groupSsh, &GroupSsh::nfyConnected, groupConfig, &GroupConfig::enableLoad);
    connect(groupSsh, &GroupSsh::nfyConnected, groupSpecs, &GroupSpecs::handleConnect);
    connect(groupSsh, &GroupSsh::nfyBitstreamLoaded, groupSpecs, &GroupSpecs::bitstreamLoaded);
    connect(groupSsh, &GroupSsh::showHelp, [=](){showHelp("sec.connect");});


    //:showTooltipSwitch
    connect(showTooltipSwitch, &Switch::toggled, this, &Window::setTooltipsVisible);
    showTooltipSwitch->animateClick(0);

#ifdef _WIN32
    //:themeSwitch
    connect(themeSwitch, &Switch::toggled, this, &Window::setDarkTheme);
    if(isDarkTheme)
        themeSwitch->animateClick();
#endif //_WIN32

    //:helpClickLabel | :introClickLabel
    connect(helpClickLabel, &ClickLabel::clicked, [=](){showHelp("sec.plot");});
    connect(introClickLabel, &ClickLabel::clicked, [=](){showHelp("sec.intro");});

    //:plotTypeCombo
    plotTypeCombo->addItem(tr("Amplitude Plot"));
    plotTypeCombo->addItem(tr("Bode Plot"));
    plotTypeCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(plotTypeCombo, &QComboBox::currentTextChanged, kerPlot, &KerPlot::setPlotType);
    plotTypeCombo->setCurrentIndex(-1);plotTypeCombo->setCurrentIndex(0);

    //:plotPerfCombo
//    plotPerfCombo->addItem(tr("High performance"));
//    plotPerfCombo->addItem(tr("High quality"));
//    plotPerfCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    connect(plotPerfCombo, &QComboBox::currentTextChanged, kerPlot, &KerPlot::setPlotPerf);
//    plotTypeCombo->setCurrentIndex(-1);plotTypeCombo->setCurrentIndex(0);

    //:srcTransShowSwitch|:firTransShowSwitch|:totalTransShowSwitch
    connect(srcTransShowSwitch, &Switch::toggled, kerPlot, &KerPlot::toggleSrcTransPlot);
    connect(firTransShowSwitch, &Switch::toggled, kerPlot, &KerPlot::toggleFirTransPlot);
    connect(totalTransShowSwitch, &Switch::toggled, kerPlot, &KerPlot::toggleTotalTransPlot);
    srcTransShowSwitch->animateClick(0);
    firTransShowSwitch->animateClick(0);
    kerPlot->toggleTotalTransPlot(false);

    //:this
    auto sh = this->sizeHint();
    resize(sh.width(), sh.height());
    setWindowTitle(tr(WINDOW_TITLE));

    groupSpecs->setFocus();

}

void Window::setDarkTheme(bool darkTheme){
    if(isDarkTheme == darkTheme)
        return;
    isDarkTheme = darkTheme;
    QPalette palette;
    QPalette dialPal = qApp->palette("QDialog");
    if(darkTheme){
        palette.setColor(QPalette::Window, XColor::base03);
        palette.setColor(QPalette::WindowText, XColor::base2);
        palette.setColor(QPalette::Text, XColor::base02);
        palette.setColor(QPalette::ButtonText, XColor::base02);

        palette.setColor(QPalette::Inactive, QPalette::WindowText, XColor::base1);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText, XColor::base01);

        QSettings appSet;
        appSet.setValue("view/darkTheme",true);
    }
    else{ //light theme

        QColor backColor = XColor::base3;
        palette.setColor(QPalette::Window, backColor);
        QColor forgrColor = XColor::base02;
        palette.setColor(QPalette::WindowText, forgrColor);
        palette.setColor(QPalette::Text, forgrColor);
        palette.setColor(QPalette::ButtonText, forgrColor);

        palette.setColor(QPalette::Inactive, QPalette::WindowText, XColor::base01);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText, XColor::base1);

        QSettings appSet;
        appSet.setValue("view/darkTheme", false);
    } //light theme end

    qApp->setPalette(palette);
    qApp->setPalette(dialPal, "QDialog");
}

void Window::setTooltipsVisible(bool v){
    if(v)
        qApp->removeEventFilter(this);
    else
        qApp->installEventFilter(this);
}

void Window::showHelp(const QString& anchor){
    helpBrowser->setSource(QString("help.html#")+ anchor);
    helpBrowser->show();
    helpBrowser->activateWindow();
}

bool Window::eventFilter(QObject* obj, QEvent* event){
    if (event->type() == QEvent::ToolTip)
        return true;

    return QMainWindow::eventFilter(obj, event);
}
