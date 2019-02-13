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

    Switch* showTooltipSwitch = new Switch(tr("Show Tooltips"));
    showTooltipSwitch->animateClick(0);
    //QCheckBox* srcTransShowSwitch = new QCheckBox("Show rate conversion transmission");
    showTooltipSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lVBox->addWidget(showTooltipSwitch);


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


    Switch* firTransShowSwitch = new Switch("Filter transmission",QBrush(QColor(31, 119, 179)));
    firTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    plCtrlHBox->addWidget(firTransShowSwitch);

    Switch* srcTransShowSwitch = new Switch("Rate conversion transmission",QBrush(QColor(249, 124, 14)));
    srcTransShowSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    plCtrlHBox->addWidget(srcTransShowSwitch);

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

    srcTransShowSwitch->animateClick(0);
    firTransShowSwitch->animateClick(0);

    connect(groupConfig, &GroupConfig::bitstreamSelected, groupSpecs, &GroupSpecs::bitstreamChanged);
    connect(groupConfig, &GroupConfig::fpgaSampFreqChanged, groupSpecs, &GroupSpecs::setFpgaSampFreq);
    groupConfig->init();

    connect(showTooltipSwitch, &Switch::toggled, this, &Window::setTooltipsVisible);


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

//QGroupBox *Window::createFirstExclusiveGroup()
//{
//    QGroupBox *groupBox = new QGroupBox(tr("Exclusive Radio Buttons"));

//    QRadioButton *radio1 = new QRadioButton(tr("&Radio button 1"));
//    QRadioButton *radio2 = new QRadioButton(tr("R&adio button 2"));
//    QRadioButton *radio3 = new QRadioButton(tr("Ra&dio button 3"));

//    radio1->setChecked(true);

//    QVBoxLayout *vbox = new QVBoxLayout;
//    vbox->addWidget(radio1);
//    vbox->addWidget(radio2);
//    vbox->addWidget(radio3);
//    vbox->addStretch(1);
//    groupBox->setLayout(vbox);

//    return groupBox;
//}

//QGroupBox *Window::createSecondExclusiveGroup()
//{
//    QGroupBox *groupBox = new QGroupBox(tr("E&xclusive Radio Buttons"));
//    groupBox->setCheckable(true);
//    groupBox->setChecked(false);

//    QRadioButton *radio1 = new QRadioButton(tr("Rad&io button 1"));
//    QRadioButton *radio2 = new QRadioButton(tr("Radi&o button 2"));
//    QRadioButton *radio3 = new QRadioButton(tr("Radio &button 3"));
//    radio1->setChecked(true);
//    QCheckBox *checkBox = new QCheckBox(tr("Ind&ependent checkbox"));
//    checkBox->setChecked(true);

//    QVBoxLayout *vbox = new QVBoxLayout;
//    vbox->addWidget(radio1);
//    vbox->addWidget(radio2);
//    vbox->addWidget(radio3);
//    vbox->addWidget(checkBox);
//    vbox->addStretch(1);
//    groupBox->setLayout(vbox);

//    return groupBox;
//}

//QGroupBox *Window::createNonExclusiveGroup()
//{
//    QGroupBox *groupBox = new QGroupBox(tr("Non-Exclusive Checkboxes"));
//    groupBox->setFlat(true);

//    QCheckBox *checkBox1 = new QCheckBox(tr("&Checkbox 1"));
//    QCheckBox *checkBox2 = new QCheckBox(tr("C&heckbox 2"));
//    checkBox2->setChecked(true);
//    QCheckBox *tristateBox = new QCheckBox(tr("Tri-&state button"));
//    tristateBox->setTristate(true);
//    tristateBox->setCheckState(Qt::PartiallyChecked);

//    QVBoxLayout *vbox = new QVBoxLayout;
//    vbox->addWidget(checkBox1);
//    vbox->addWidget(checkBox2);
//    vbox->addWidget(tristateBox);
//    vbox->addStretch(1);
//    groupBox->setLayout(vbox);

//    return groupBox;
//}

//QGroupBox *Window::createPushButtonGroup()
//{
//    QGroupBox *groupBox = new QGroupBox(tr("&Push Buttons"));
//    groupBox->setCheckable(true);
//    groupBox->setChecked(true);

//    QPushButton *pushButton = new QPushButton(tr("&Normal Button"));
//    QPushButton *toggleButton = new QPushButton(tr("&Toggle Button"));
//    toggleButton->setCheckable(true);
//    toggleButton->setChecked(true);
//    QPushButton *flatButton = new QPushButton(tr("&Flat Button"));
//    flatButton->setFlat(true);

//    QPushButton *popupButton = new QPushButton(tr("Pop&up Button"));
//    QMenu *menu = new QMenu(this);
//    menu->addAction(tr("&First Item"));
//    menu->addAction(tr("&Second Item"));
//    menu->addAction(tr("&Third Item"));
//    menu->addAction(tr("F&ourth Item"));
//    popupButton->setMenu(menu);

//    QAction *newAction = menu->addAction(tr("Submenu"));
//    QMenu *subMenu = new QMenu(tr("Popup Submenu"));
//    subMenu->addAction(tr("Item 1"));
//    subMenu->addAction(tr("Item 2"));
//    subMenu->addAction(tr("Item 3"));
//    newAction->setMenu(subMenu);

//    QVBoxLayout *vbox = new QVBoxLayout;
//    vbox->addWidget(pushButton);
//    vbox->addWidget(toggleButton);
//    vbox->addWidget(flatButton);
//    vbox->addWidget(popupButton);
//    vbox->addStretch(1);
//    groupBox->setLayout(vbox);

//    return groupBox;
//}

// QProcess process_;
// connect(button_, SIGNAL(released()), this, SLOT(onButtonReleased()));
// connect(&process_, SIGNAL(readyReadStandardOutput()), this, SLOT(onCaptureProcessOutput()));
// void SomeWidget::onButtonReleased(){process_.start("date");}
// void SomeWidget::onCaptureProcessOutput()
// {
//    QProcess* process = qobject_cast<QProcess*>(sender());
//    if (process) //sprawdz, czy cast sie udal
//        sometextBrowser_->insertPlainText(process->readAll()); //zrob cos
// }
