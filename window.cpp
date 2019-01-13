#include <QtWidgets>

#include "window.h"
#include "groupssh.h"
#include "groupconfig.h"
#include "groupspecs.h"
#include "kerplot.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout* mainHBox = new QHBoxLayout;

    //--- left side


    QVBoxLayout* lVBox = new QVBoxLayout;

    GroupSpecs* groupSpecs = new GroupSpecs;
    groupSpecs->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    lVBox->addWidget(groupSpecs);

    lVBox->addItem(new QSpacerItem(0,0,QSizePolicy::Preferred, QSizePolicy::Expanding));

    GroupSsh* groupSsh = new GroupSsh;
    groupSsh->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    lVBox->addWidget(groupSsh);

//    QFrame* line = new QFrame(); line->setFrameShape(QFrame::HLine); line->setFrameShadow(QFrame::Sunken);
//    lVBox->addWidget(line);

    GroupConfig* groupConfig = new GroupConfig;
    groupConfig->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lVBox->addWidget(groupConfig);


    mainHBox->addLayout(lVBox);


    //--- right side
    QVBoxLayout* rVBox = new QVBoxLayout;

    QComboBox* plotTypeCombo = new QComboBox;
    plotTypeCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rVBox->addWidget(plotTypeCombo);

    KerPlot* kerPlot = new KerPlot;
    kerPlot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    kerPlot->setMinimumSize(300, 200);
    rVBox->addWidget(kerPlot);

    mainHBox->addLayout(rVBox);


    setLayout(mainHBox);

    //---> Functionality <---//
    connect(groupSpecs, &GroupSpecs::kernelChanged, kerPlot, &KerPlot::setKernel);

    plotTypeCombo->addItem(tr("Amplitude Plot"));
    plotTypeCombo->addItem(tr("Bode Plot"));

    connect(plotTypeCombo, &QComboBox::currentTextChanged, kerPlot, &KerPlot::setPlotType);
    plotTypeCombo->setCurrentIndex(-1);plotTypeCombo->setCurrentIndex(0);


    resize(1024, 500);

    setWindowTitle(tr("FIR controller"));
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
