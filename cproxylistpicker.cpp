#include "cproxylistpicker.h"

CProxyListPicker::CProxyListPicker(QObject *parent) : QObject(parent)
{

}

void CProxyListPicker::setItems(const QStringList &items)
{
    _items = items;
    selectIndex(0);
}

void CProxyListPicker::selectIndex(int index)
{
    if(index < 0)
        index = -1;
    if(index >= _items.size())
        index = _items.size()-1;
    emit selectedText(_items.value(index));
    emit selectedIndex(index);
}
