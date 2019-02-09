#ifndef GROUPSSH_H
#define GROUPSSH_H

#include <QGroupBox>
#include <QLineEdit>
#include <QTimer>
#include "switch.h"
#include "ssh.h"

class IMLineEdit : public QLineEdit{
    Q_OBJECT
private:
    void focusInEvent(QFocusEvent *e){ QLineEdit::focusInEvent(e);
    QTimer::singleShot(0, this, [=](){setCursorPosition(0);} ); }
};


class GroupSsh : public QGroupBox
{
    Q_OBJECT
public:


    explicit GroupSsh(QWidget *parent = 0);

private:
    bool enableAdvState;
    Switch* advButton;
    Ssh ssh;
    IMLineEdit* idLineEdit;

    void toggleEnableAdv();
    void onConnect();

signals:

public slots:

};

#endif // GROUPSSH_H
