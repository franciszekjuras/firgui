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
#include "clickablelabel.h"

Window::Window(QWidget *parent)
    : QMainWindow(parent)
{
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

        GroupSpecs* groupSpecs = new GroupSpecs;
        lVBox->addWidget(groupSpecs);
        groupSpecs->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

        GroupConfig* groupConfig = new GroupConfig;
        lVBox->addWidget(groupConfig);
        groupConfig->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

        GroupSsh* groupSsh = new GroupSsh;
        lVBox->addWidget(groupSsh);
        groupSsh->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);

        lVBox->addItem(new QSpacerItem(0,0,QSizePolicy::Preferred, QSizePolicy::Expanding));

//        BoxUpdate* boxUpdate = new BoxUpdate;
//        lVBox->addWidget(boxUpdate);

        QWidget* lbotWid = new QWidget;
        lVBox->addWidget(lbotWid);
        QHBoxLayout* lbotHBox = new QHBoxLayout;
        lbotWid->setLayout(lbotHBox);

            Switch* showTooltipSwitch = new Switch(tr("Show Tooltips"));
            //lbotHBox->addWidget(showTooltipSwitch);
            showTooltipSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            showTooltipSwitch->setFocusPolicy(Qt::ClickFocus);


            //lbotHBox->addItem(new QSpacerItem(5,0,QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));

            ClickableLabel* decrText = new ClickableLabel();
            decrText->setText("<p><small>A<sup>―</sup></small></p>");
            lbotHBox->addWidget(decrText);
            connect(decrText, &ClickableLabel::clicked, [=](){
                QFont appFont = QApplication::font();
                if(appFont.pointSize()>5)
                    {appFont.setPointSize(appFont.pointSize()-1);}
                QApplication::setFont(appFont);});

            ClickableLabel* incrText = new ClickableLabel();
            incrText->setText("<p><big>A<sup>+</sup></big></p>");
            lbotHBox->addWidget(incrText);
            connect(incrText, &ClickableLabel::clicked, [=](){
                QFont appFont = QApplication::font();
                appFont.setPointSize(appFont.pointSize()+1);
                QApplication::setFont(appFont);});

            lbotHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred));

            lbotHBox->addWidget(new QLabel(QString("v") + VERSIONSTRING));


    QWidget* rWid = new QWidget;
    mainHBox->addWidget(rWid);
    rWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout* rVBox = new QVBoxLayout;
    rWid->setLayout(rVBox);
    rVBox->setContentsMargins(0,0,0,0);
    //rVBox --v

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

            QComboBox* plotTypeCombo = new QComboBox;            
#ifdef _WIN32
        plotTypeCombo->view()->setItemDelegate(new PopupItemDelegate(plotTypeCombo));
#endif
            plCtrlHBox->addWidget(plotTypeCombo);
            plotTypeCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    KerPlot* kerPlot = new KerPlot;
    rVBox->addWidget(kerPlot);
    kerPlot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

//part:funtion

    //:groupSpecs
    connect(groupSpecs, &GroupSpecs::kernelChanged, kerPlot, &KerPlot::setKernel);
//    connect(groupSpecs, &GroupSpecs::kernelClear, kerPlot, &KerPlot::clearKernel);
    connect(groupSpecs, &GroupSpecs::srcKernelChanged, kerPlot, &KerPlot::setSrcKernel);
//    connect(groupSpecs, &GroupSpecs::srcKernelClear, kerPlot, &KerPlot::clearSrcKernel);
    connect(groupSpecs, &GroupSpecs::resetPlot, kerPlot, &KerPlot::resetPlot);
    connect(groupSpecs, &GroupSpecs::reqLoadSrcKernel, groupSsh, &GroupSsh::loadSrcKernel);
    connect(groupSpecs, &GroupSpecs::reqLoadKernel, groupSsh, &GroupSsh::loadKernel);

    //:groupConfig
    connect(groupConfig, &GroupConfig::bitstreamSelected, groupSpecs, &GroupSpecs::bitstreamChanged);
    connect(groupConfig, &GroupConfig::fpgaSamplingFreqChanged, groupSpecs, &GroupSpecs::setfpgaSamplingFreq);
    connect(groupConfig, &GroupConfig::reqLoad, groupSsh, &GroupSsh::onLoad);
    groupConfig->init();

    //:groupSsh
    connect(groupSsh, &GroupSsh::nfyConnected, groupConfig, &GroupConfig::enableLoad);
    connect(groupSsh, &GroupSsh::nfyConnected, groupSpecs, &GroupSpecs::handleConnect);
    connect(groupSsh, &GroupSsh::nfyBitstreamLoaded, groupSpecs, &GroupSpecs::bitstreamLoaded);


    //:showTooltipSwitch
    connect(showTooltipSwitch, &Switch::toggled, this, &Window::setTooltipsVisible);
    showTooltipSwitch->animateClick(0);

    //:plotTypeCombo
    plotTypeCombo->addItem(tr("Amplitude Plot"));
    plotTypeCombo->addItem(tr("Bode Plot"));
    connect(plotTypeCombo, &QComboBox::currentTextChanged, kerPlot, &KerPlot::setPlotType);
    plotTypeCombo->setCurrentIndex(-1);plotTypeCombo->setCurrentIndex(0);

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



}

void Window::setTooltipsVisible(bool v){
    if(v)
        qApp->removeEventFilter(this);
    else
        qApp->installEventFilter(this);
}

bool Window::eventFilter(QObject* obj, QEvent* event){
    if (event->type() == QEvent::ToolTip)
        return true;

    return QMainWindow::eventFilter(obj, event);
}
