#ifndef GROUPCONFIG_H
#define GROUPCONFIG_H

#include <QGroupBox>

class QComboBox;

class GroupConfig : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupConfig(QWidget *parent = 0);

private:
    QComboBox* bitCombo;

signals:

public slots:
};

#endif // GROUPCONFIG_H
