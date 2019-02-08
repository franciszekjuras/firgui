#ifndef BOOLMAPWATCHER_H
#define BOOLMAPWATCHER_H

#include <QObject>
#include <map>
#include <QString>

class BoolMapOr : public QObject
{
    Q_OBJECT
private:
    std::map<QString, bool> _map;
    bool _en;

    void update();

public:
    explicit BoolMapOr(QObject *parent = 0);

signals:
    void valueChanged(bool en);

public slots:
    void enable(QString idx);
    void disable(QString idx);
};

#endif // BOOLMAPWATCHER_H
