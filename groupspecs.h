#ifndef GROUPSPECS_H
#define GROUPSPECS_H

#include <QGroupBox>
#include <string>
#include <vector>
#include "firker.h"

class QLineEdit;

class GroupSpecs : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupSpecs(QWidget *parent = 0);

private:
    QLineEdit* freqsLineEdit;
    QLineEdit* gainsLineEdit;
    std::vector<double> crrKer;

    static bool textToDoubles(const std::string& str, std::vector<double>& v);

public slots:
    void calculateKernel();

signals:
    void kernelChanged(const FirKer& ker);


};

#endif // GROUPSPECS_H
