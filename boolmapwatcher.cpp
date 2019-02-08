#include "boolmapwatcher.h"
#include <algorithm>
#include <QDebug>

BoolMapOr::BoolMapOr(QObject *parent) : QObject(parent)
{
    _en = false;
}


void BoolMapOr::enable(QString idx){
    _map[idx] = true;
    update();
}

void BoolMapOr::disable(QString idx){
    _map[idx] = false;
    update();
}

void BoolMapOr::update(){
    bool en = std::any_of(_map.cbegin(), _map.cend(), [](auto v){return v.second;});
    if(_en != en){
        _en = en;
        emit valueChanged(en);
    }
}
