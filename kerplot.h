#ifndef PLOT_H
#define PLOT_H

#include <QVector>
#include <QString>
#include <QFutureWatcher>
#include <QTimer>
#include <memory>
#include "qcustomplot.h"
#include "firker.h"
#include "waitingspinnerwidget.h"
#include "boolmapwatcher.h"

#define KERPLOT_FOCUS_DIST 12.5

class KerPlot : public QCustomPlot{
    Q_OBJECT

public:
    KerPlot(QWidget* parent = nullptr);
    //~Plot();

    QSize sizeHint() const;
protected:
    enum class SpecFocus {
        none,
        band,
        gain
    };

    SpecFocus specFocusType;
    int specFIdx;
    int specGIdx;
    bool specMouseCapt;
    QPoint curPos;
    QMouseEvent lastMoveEvent;

    QCPRange xRange;
    double kerMaxGain;
    double srcKerMaxGain;
    QString plotType;
    int srcPlotDiv;
    int plotDiv;
    int plotDivScale;
    int t;
    int band;

    //this values are for display only
    bool inverseBand;
    double lBandLimit;
    double rBandLimit;
    double nqFreq;

    QVector<double> transmission;
    QVector<double> transmissionBode;
    QVector<double> srcTransmission;
    QVector<double> srcTransmissionBode;
    QVector<double> freqs;
    QVector<double> srcFreqs;
    QVector<double> specFreqs;
    QVector<double> specGains;
    QVector<double> specTransFreqs;
    QVector<double> specTransGains;
    QVector<double> highlightFreqs;
    QVector<double> highlightGains;


    QFutureWatcher<std::vector<double> > kerTransWatch;
    QFutureWatcher<std::vector<double> > srcKerTransWatch;
    bool plotClearedMeanwhile;
    bool srcPlotClearedMeanwhile;
    WaitingSpinnerWidget* waitSpin;    
    BoolMapOr spinWatch;
    double unitMult;
    QTimer perfModeTimer;
    double penWidth;
    double highlightWidth;
    int perfModeTimeoutMS;

    //bool eventFilter(QObject *obj, QEvent *event);


    void mouseMoveEvent(QMouseEvent *event);
    void mouseMoveEventHandler(QMouseEvent *event, bool insideEvent = false);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void leaveEvent(QEvent* event);


    bool checkFocusGrab();
    bool checkFocusLost();
    void handleFocusGrab();
    void handleFocusLost();

    inline double xCTP(double v);
    inline double yCTP(double v);
    inline double xPTC(double v);
    inline double yPTC(double v);

    void highlightFocus();
    void removeFocusHighlight();

    void calcSpecTrans();

    void amplitudePlot();
    void bodePlot();
    void setFreqs(double freq, int t, int band);
    void updateFreqs();
    void updateSrcFreqs();
    void totalAmplTrans();
    void totalBodeTrans();
    void clearTotalTrans();
    void exitPerfMode();
    void enterPerfMode();

    void cntSetKernel();
    void cntSetSrcKernel();

public slots:
    void checkXBounds(const QCPRange& newRange);
    void setSpec(QVector<double> freqs, QVector<double> gains);
    void setKernel(std::shared_ptr<const FirKer> kernel, double roiL, double roiR);
    void setSrcKernel(std::shared_ptr<const FirKer> kernel);
    void setPlotType(const QString& plotType);
    //void setPlotPerf(const QString& plotType);
    void resetPlot(double freq, int t, int band);
    void clearKernel();
    void clearSrcKernel();
    void clearSpec();
    void toggleTotalTransPlot(bool);
    void toggleSrcTransPlot(bool);
    void toggleFirTransPlot(bool);
    void toggleSpecTransPlot(bool toggle);
    bool isSpecEditEn();

};

#endif // PLOT_H
