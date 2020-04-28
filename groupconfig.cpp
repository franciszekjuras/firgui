#include <QtWidgets>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QMap>
#include <vector>
#include <algorithm>
#include <cassert>
#include "groupconfig.h"
#include "bitstreamspecs.h"
#ifdef _WIN32
#include "delegate.h"
#endif
#include "xcolor.h"
#include "clicklabel.h"

#ifdef _WIN32

GroupConfig::GroupConfig(QWidget *parent) : QWidget(parent)
{

//part:layout
QVBoxLayout* groupVBox = new QVBoxLayout;
this->setLayout(groupVBox);

QLabel* titleLabel = new QLabel(QString("<b>") + tr("Configuration") + "</b>");
titleLabel->setContentsMargins(5,0,0,5);
groupVBox->addWidget(titleLabel);
QPalette pal;
pal.setColor(titleLabel->foregroundRole(),pal.color(QPalette::Inactive, QPalette::WindowText));
titleLabel->setPalette(pal);
connect(this, &GroupConfig::updatePalette, [=](){QPalette pal;
    pal.setColor(titleLabel->foregroundRole(),pal.color(QPalette::Inactive, QPalette::WindowText));
    titleLabel->setPalette(pal);});
qApp->installEventFilter(this);
//neccessary to keep up with palette changes

QWidget* groupContent = new QWidget;
groupVBox->addWidget(groupContent);

QVBoxLayout* mainVBox = new QVBoxLayout;
mainVBox->setContentsMargins(0,0,0,0);
groupContent->setLayout(mainVBox);
#else

GroupConfig::GroupConfig(QWidget *parent) : QGroupBox(tr("Configuration"),parent)
{
//part:layout

setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
QVBoxLayout* mainVBox = new QVBoxLayout;
this->setLayout(mainVBox);
#endif
//mainVBox --v

    QFormLayout* bitstreamForm = new QFormLayout;
    mainVBox->addLayout(bitstreamForm);
    //bitstreamForm --v

        bandwidthCombo = new QComboBox;
#ifdef _WIN32
        bandwidthCombo->view()->setItemDelegate(new PopupItemDelegate(bandwidthCombo));
#endif
        bitstreamForm->addRow(tr("Working band width"), bandwidthCombo);
        bandwidthCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        bandwidthCombo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        bandwidthCombo->setToolTip("Roll-off decreases qudratically with working band width.");

        rankCombo = new QComboBox;
#ifdef _WIN32
        rankCombo->view()->setItemDelegate(new PopupItemDelegate(rankCombo));
#endif
        QString rankComboName = tr("Filter Rank") + " | " + tr("SRC blocks");
        bitstreamForm->addRow(rankComboName, rankCombo);
        rankCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        rankCombo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        rankCombo->setToolTip(tr("Higher number of SRC blocks gives sharper rate conversion transmission on cost of reduced filter rank."));

    QHBoxLayout* buttonHBox = new QHBoxLayout;
    mainVBox->addLayout(buttonHBox);
    //buttonHBox --v

        ClickLabel* helpClickLabel = new ClickLabel("<big>?</big>");
        buttonHBox->addWidget(helpClickLabel);

        buttonHBox->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Preferred));

        QPushButton* loadButton = new QPushButton(tr("Load"));
        buttonHBox->addWidget(loadButton);
        loadButton->setDisabled(true);


//part:function

    //:this
    connect(this, &GroupConfig::enableLoad, loadButton, &QPushButton::setEnabled);

    //:helpClickLabel
    connect(helpClickLabel, &ClickLabel::clicked, this, &GroupConfig::showHelp);

    //:loadButton
    connect(loadButton, &QPushButton::clicked, this, &GroupConfig::onLoadButton);

}

void GroupConfig::onLoadButton(){
    qInfo() << "Load clicked";
    reqLoad(crrBitstream);
}

void GroupConfig::init(){

//:bandwidthCombo|:rankCombo

    fpgaSamplingFreq = 125000.;
    emit fpgaSamplingFreqChanged(fpgaSamplingFreq);

    QDir bitDir("data/bitstreams");
    bitDir.setFilter(QDir::Files);
    bitDir.setNameFilters(QStringList("*.bin"));
    QFileInfoList binFiles = bitDir.entryInfoList();

    std::vector<BitstreamSpecs> bitSpecsV;
    for(const auto& fn : binFiles){
        BitstreamSpecs bitSpec(fn);
        if(bitSpec.isValid() && bitSpec.getSpecs().contains("tm"))
            bitSpecsV.push_back(bitSpec);
    }

    if(bitSpecsV.empty()){
        qCritical() << "No valid bitstreams found.";
        QMessageBox::critical(this, tr(WINDOW_TITLE), tr("No valid bitstreams found. Possible reasons:\n- executable file was moved or distributed without \"data\" folder,\n- application wasn't launched from directory where executable is placed."));
        return;
    }


    auto bitSortF = [](const BitstreamSpecs& a, const BitstreamSpecs& b){
        int atm = a.getSpecs().value("tm",0);
        int btm = b.getSpecs().value("tm",0);
        if(atm < btm) return true;
        if(atm > btm) return false;
        int afb = a.getSpecs().value("fb",0);
        int bfb = b.getSpecs().value("fb",0);
        if(afb < bfb) return true;
        return false;
    };

    std::sort(bitSpecsV.begin(),bitSpecsV.end(), bitSortF);

    QString key1P;
    for(const auto& bitSpec : bitSpecsV){
        auto params = bitSpec.getSpecs();
        double nyqFreq = fpgaSamplingFreq/2./ static_cast<double>(params["tm"]);
        QString key1;
        if(nyqFreq >= 1000.) key1 = QString::number(nyqFreq/1000.,'g',4)+" MHz";
        else key1 = QString::number(nyqFreq,'g',4)+" KHz";
        QString key2;

        if(params.contains("fb"))
            key2 += QString::number(params["fb"]*params["tm"]);
        else
            key2 += tr("unk.");

        key2 += " | ";

        if(params.contains("sb"))
            key2 += QString::number(params["sb"]);
        else
            key2 += tr("unk.");

        if(key1 != key1P){
            bandwidthCombo->addItem(key1);
            key1P = key1;
        }

        bitMap[key1][key2] = bitSpec;
    }

    bandwidthCombo->setCurrentIndex(-1);
    connect(bandwidthCombo, &QComboBox::currentTextChanged, this, &GroupConfig::bandwidthComboChanged);
    connect(rankCombo, &QComboBox::currentTextChanged, this, &GroupConfig::rankComboChanged);
    bandwidthCombo->setCurrentIndex(0);
}

void GroupConfig::bandwidthComboChanged(QString mainStr){
    assert(bitMap.contains(mainStr));
    qInfo() << "Bandwidth combo changed to:" << mainStr;
    updateRankCombo(mainStr);
}

void GroupConfig::updateRankCombo(QString mainStr){
    bitMainStr = mainStr;
    rankCombo->clear();
    auto specMap = bitMap.value(mainStr);
    for(auto it = specMap.cend(); it != specMap.cbegin();)
        rankCombo->addItem((--it).key());
}

void GroupConfig::rankComboChanged(QString specStr){
    if(specStr.isEmpty()) return;
    assert(bitMap.value(bitMainStr).contains(specStr));
    qInfo() << "Rank combo changed to:" << specStr;
    crrBitstream = bitMap[bitMainStr][specStr];
    emit bitstreamSelected(crrBitstream.getSpecs());
}

bool GroupConfig::eventFilter(QObject* obj, QEvent* event){
    if (event->type() == QEvent::PaletteChange && obj == this)
        updatePalette();


    return QWidget::eventFilter(obj, event);
}
