#include <QtWidgets>

#include "window.h"
#include "groupssh.h"
#include "groupconfig.h"
#include "groupspecs.h"
#include "kerplot.h"
#include "switch.h"

Window::Window(QWidget *parent)
    : QMainWindow(parent)
{
    QHBoxLayout* mainHBox = new QHBoxLayout;

    //--- left side

    QWidget* lWid = new QWidget;

    lWid->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
     //lWid->setMaximumWidth(400);

    QVBoxLayout* lVBox = new QVBoxLayout;
    lVBox->setContentsMargins(0,0,0,0);

    GroupSpecs* groupSpecs = new GroupSpecs;
    groupSpecs->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lVBox->addWidget(groupSpecs);

    GroupConfig* groupConfig = new GroupConfig;
    groupConfig->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lVBox->addWidget(groupConfig);

    GroupSsh* groupSsh = new GroupSsh;
    groupSsh->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lVBox->addWidget(groupSsh);

    lVBox->addItem(new QSpacerItem(0,0,QSizePolicy::Preferred, QSizePolicy::Expanding));

    QWidget* lbotWid = new QWidget;
    QHBoxLayout* lbotHBox = new QHBoxLayout;

    Switch* showTooltipSwitch = new Switch(tr("Show Tooltips"));
    showTooltipSwitch->animateClick(0);
    showTooltipSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbotHBox->addWidget(showTooltipSwitch);

    lbotHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred));
    lbotHBox->addWidget(new QLabel("v1.0.5dev"));

    lbotWid->setLayout(lbotHBox);
    lVBox->addWidget(lbotWid);

//    QFrame* line = new QFrame(); line->setFrameShape(QFrame::HLine); line->setFrameShadow(QFrame::Sunken);
//    lVBox->addWidget(line);


    lWid->setLayout(lVBox);
    mainHBox->addWidget(lWid);


    //--- right side
    QVBoxLayout* rVBox = new QVBoxLayout;

    rVBox->setContentsMargins(0,0,0,0);

    QWidget* plCtrlWid = new QWidget;
    plCtrlWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QHBoxLayout* plCtrlHBox = new QHBoxLayout;
    plCtrlHBox->setContentsMargins(0,0,0,0);
    plCtrlWid->setLayout(plCtrlHBox);


    Switch* firTransShowSwitch = new Switch(tr("Filter transmission"),QBrush(QColor(132,186,91)));
    firTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    plCtrlHBox->addWidget(firTransShowSwitch);

    Switch* srcTransShowSwitch = new Switch(tr("Rate conversion trans."),QBrush(QColor(249, 124, 14)));
    srcTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    plCtrlHBox->addWidget(srcTransShowSwitch);

    Switch* totalTransShowSwitch = new Switch(tr("Total trans."),QBrush(QColor(114,147,203)));
    totalTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    plCtrlHBox->addWidget(totalTransShowSwitch);

    plCtrlHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

    QComboBox* plotTypeCombo = new QComboBox;
    plotTypeCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    plCtrlHBox->addWidget(plotTypeCombo);


    rVBox->addWidget(plCtrlWid);

    KerPlot* kerPlot = new KerPlot;
    kerPlot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    //kerPlot->setMinimumSize(300, 200);
    rVBox->addWidget(kerPlot);

    mainHBox->addLayout(rVBox);

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainHBox);
    setCentralWidget(centralWidget);

    //---> Functionality <---//
    connect(groupSpecs, &GroupSpecs::kernelChanged, kerPlot, &KerPlot::setKernel);
    connect(groupSpecs, &GroupSpecs::kernelClear, kerPlot, &KerPlot::clearKernel);
    connect(groupSpecs, &GroupSpecs::srcKernelChanged, kerPlot, &KerPlot::setSrcKernel);
    connect(groupSpecs, &GroupSpecs::srcKernelClear, kerPlot, &KerPlot::clearSrcKernel);
    connect(groupSpecs, &GroupSpecs::resetPlot, kerPlot, &KerPlot::resetPlot);

    plotTypeCombo->addItem(tr("Amplitude Plot"));
    plotTypeCombo->addItem(tr("Bode Plot"));

    connect(plotTypeCombo, &QComboBox::currentTextChanged, kerPlot, &KerPlot::setPlotType);
    plotTypeCombo->setCurrentIndex(-1);plotTypeCombo->setCurrentIndex(0);
    connect(srcTransShowSwitch, &Switch::toggled, kerPlot, &KerPlot::toggleSrcTransPlot);
    connect(firTransShowSwitch, &Switch::toggled, kerPlot, &KerPlot::toggleFirTransPlot);
    connect(totalTransShowSwitch, &Switch::toggled, kerPlot, &KerPlot::toggleTotalTransPlot);

    srcTransShowSwitch->animateClick(0);
    firTransShowSwitch->animateClick(0);
    kerPlot->toggleTotalTransPlot(false);
    //totalTransShowSwitch->animateClick(0);


    connect(groupConfig, &GroupConfig::bitstreamSelected, groupSpecs, &GroupSpecs::bitstreamChanged);
    connect(groupConfig, &GroupConfig::fpgaSampFreqChanged, groupSpecs, &GroupSpecs::setFpgaSampFreq);
    groupConfig->init();

    connect(showTooltipSwitch, &Switch::toggled, this, &Window::setTooltipsVisible);

    //RP connection:
    connect(groupSsh, &GroupSsh::nfyConnected, groupConfig, &GroupConfig::enableLoad);
    connect(groupSsh, &GroupSsh::nfyConnected, groupSpecs, &GroupSpecs::handleConnect);
    connect(groupConfig, &GroupConfig::reqLoad, groupSsh, &GroupSsh::onLoad);
    connect(groupSsh, &GroupSsh::nfyBitstreamLoaded, groupSpecs, &GroupSpecs::bitstreamLoaded);
    connect(groupSpecs, &GroupSpecs::reqLoadSrcKernel, groupSsh, &GroupSsh::loadSrcKernel);
    connect(groupSpecs, &GroupSpecs::reqLoadKernel, groupSsh, &GroupSsh::loadKernel);



    auto sh = this->sizeHint();

    resize(sh.width(), sh.height());

    setWindowTitle(tr("FIR Controller"));

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
