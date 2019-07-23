#ifndef GROUPSSH_H
#define GROUPSSH_H

#include <vector>
#include <QGroupBox>
#include <QLineEdit>
#include <QTimer>
#include <QFutureWatcher>
#include "switch.h"
#include "ssh.h"
#include "bitstreamspecs.h"
#include "waitingspinnerwidget.h"

#ifndef SSH_TIMEOUT
#define SSH_TIMEOUT=10L
#endif

class IMLineEdit : public QLineEdit{
    Q_OBJECT
private:
    void focusInEvent(QFocusEvent *e){ QLineEdit::focusInEvent(e);
    QTimer::singleShot(0, this, [=](){setCursorPosition(0);} ); }
};


class GroupSsh : public QGroupBox
{
    Q_OBJECT

private:
    enum class R {
        ok,
        connection,
        auth,
        other
    };

public:
    explicit GroupSsh(QWidget *parent = 0);

private:
    bool enableAdvState;
    Switch* advButton;
    QPushButton* connectButton;
    QPushButton* disconnectButton;
    Ssh ssh;
    IMLineEdit* idLineEdit;
    QFutureWatcher<R> connectWatch;
    QFutureWatcher<R> authWatch;
    WaitingSpinnerWidget* waitSpin;
    int fcMajVer;
    int fcSubVer;
    bool fcUploaded;

    void swapConnectButtons(bool isConnect);
    void onConnect();
    void onDisconnect();

    R loadBitstream(BitstreamSpecs bitSpecs);

    R connectToRP(std::string rpMac);
    void connectToRPFinished();
    R authenticateRP(std::string pass);
    void authenticateRPFinished();
    R uploadFirCtrl(const BitstreamSpecs& bitSpecs);

signals:
    void nfyBitstreamLoaded(QMap<QString, int> specs);
    void nfyConnected(bool is);

public slots:
    void onLoad(BitstreamSpecs bitSpecs);
    void loadSrcKernel(std::vector<double> crrSrcKer);
    void loadKernel(std::vector<double> crrKer);

};

//"  [-tm | --time-multiplexing] Time multiplexing rank\n"
//"  [-cs | --coef-size] Single coefficient byte-size\n"
//"  [-sr | --src-rank] Sampling rate conversion kernel rank\n"
//"  [-fr | --fir-rank] FIR kernel rank\n"
//"  [-sb | --src-blocks] Sampling rate conversion blocks per one conversion\n"
//"  [-fb | --fir-blocks] FIR blocks\n"
//"  [-sp | --src-precision] Sampling rate conversion kernel fixed point precision\n"
//"  [-fp | --fir-precision] FIR fixed point precision\n"
//"  --load:      Load kernel\n"
//"  --enable:    Enable filter\n"
//"  --disable:   Disable filter\n"
//"  --zero:      Set all coefficients to 0\n"
//"  --info:      Show info\n"

#endif // GROUPSSH_H
