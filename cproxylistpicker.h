#ifndef CPROXYLISTPICKER_H
#define CPROXYLISTPICKER_H

#include <QObject>
#include <QStringList>

class CProxyListPicker : public QObject
{
    Q_OBJECT
public:
    explicit CProxyListPicker(QObject *parent = nullptr);

    void setItems(const QStringList &data);

signals:
    void selectedText(QString text);
    void selectedIndex(int index);

public slots:
    void selectIndex(int index);

protected:
    QStringList _items;


};

#endif // CPROXYLISTPICKER_H
