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

    QString version = nameSplit.at(0);
    if(!setVersion(version)) return;

    specs = mapSpecs(nameSplit.at(1));
    if (specs.isEmpty()) return;

    for(int i = 2; i < nameSplit.size(); ++i){
        comment += nameSplit.at(i);
        comment += " ";
    }
    comment.chop(1); //get rid of extra space

    valid = true;
}

bool BitstreamSpecs::isValid() const{return valid;}
const QMap<QString, int>& BitstreamSpecs::getSpecs() const{return specs;}
QString BitstreamSpecs::getFileName() const{return fileInfo.fileName();}
QString BitstreamSpecs::getFilePath() const{return fileInfo.filePath();}
const QString& BitstreamSpecs::getComment() const{return comment;}
int BitstreamSpecs::getMajVersion() const{return majVersion;}
int BitstreamSpecs::getSubVersion() const{return subVersion;}

bool BitstreamSpecs::setVersion(const QString& str){
    if(!str.startsWith("fir",Qt::CaseInsensitive))
        return false;
    QString ver = str.mid(3);
    QStringList verSplit = ver.split('v');
    if(verSplit.size()!=2) return false;
    int majV, subV;
    bool ok;
    majV = verSplit.at(0).toInt();
    subV = verSplit.at(1).toInt(&ok);
    if(majV <= 0 || subV < 0 || (!ok))
        return false;
    majVersion = majV;
    subVersion = subV;
    return true;
}

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
