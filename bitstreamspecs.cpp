#include <QString>
#include <QFile>
#include <QMap>
#include "bitstreamspecs.h"

BitstreamSpecs::BitstreamSpecs(){
    valid = false;
}

BitstreamSpecs::BitstreamSpecs(const QString& filePath):fileInfo(filePath){
    init();
}

BitstreamSpecs::BitstreamSpecs(const QFileInfo& fileInfo):fileInfo(fileInfo){
    init();
}

void BitstreamSpecs::init(){
    valid = false;
    QString filename = fileInfo.fileName();

    QStringList extSplit = filename.split(".");
    if(extSplit.size()!=2 || extSplit.at(1) != "bin") return;

    QStringList nameSplit = extSplit.at(0).split("_");//<--- underscore
    if(nameSplit.size() < 2) return;

    version = nameSplit.at(0);
    if(version.isEmpty()) return;

    specs = mapSpecs(nameSplit.at(1));
    if (specs.isEmpty()) return;

    for(int i = 2; i < nameSplit.size(); ++i){
        comment += nameSplit.at(i);
        comment += " ";
    }
    comment.chop(1); //get rid of extra space

    valid = true;
}

bool BitstreamSpecs::isValid(){return valid;}
const QMap<QString, int>& BitstreamSpecs::getSpecs(){return specs;}
QString BitstreamSpecs::getFileName(){return fileInfo.fileName();}
QString BitstreamSpecs::getFilePath(){return fileInfo.filePath();}
const QString& BitstreamSpecs::getComment(){return comment;}
const QString& BitstreamSpecs::getVersion(){return version;}

QMap<QString, int> BitstreamSpecs::mapSpecs(const QString& str){
    QMap<QString, int> map;
    int pos = 0, loff = 0, noff = 0;
    while(pos != str.size()){
        loff = letterCount(str, pos);
        noff = numberCount(str, pos+loff);
        if(loff == 0 || noff == 0) return QMap<QString, int>();
        map.insert(str.mid(pos,loff), str.mid(pos + loff, noff).toInt());
        pos += loff + noff;
    }
    return map;
}

int BitstreamSpecs::letterCount(const QString& str, int pos){
    int i = pos;
    for(;(i<str.size())&&(str.at(i).isLetter()); ++i);
    return i - pos;
}

int BitstreamSpecs::numberCount(const QString& str, int pos){
    int i = pos;
    for(;(i<str.size())&&(str.at(i).isNumber()); ++i);
    return i - pos;
}
