#ifndef GROUPSSH_H
#define GROUPSSH_H

#include <vector>
#include <QGroupBox>
#include <QLineEdit>
#include <QTimer>
#include "switch.h"
#include "ssh.h"
#include "bitstreamspecs.h"

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

    void toggleEnableAdv();
    void swapConnectButtons(bool isConnect);
    void onConnect();

    R connectToRP(std::string rpMac);
    R loadBitstream(BitstreamSpecs bitSpecs);

signals:
    void reqEnableLoad(bool en);
    void nfyBitstreamLoaded(QMap<QString, int> specs);

public slots:
    void onLoad(BitstreamSpecs bitSpecs);
    void loadSrcKernel(std::vector<double> crrSrcKer);
    void loadKernel(std::vector<double> crrKer);

};

#endif // GROUPSSH_H
