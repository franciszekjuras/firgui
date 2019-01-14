#ifndef BITSTREAMSPECS_H
#define BITSTREAMSPECS_H

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QMap>


class BitstreamSpecs
{
public:
    BitstreamSpecs();
    BitstreamSpecs(const QString& filePath);
    BitstreamSpecs(const QFileInfo& fileInfo);

    bool isValid();
    const QMap<QString, int>& getSpecs();
    QString getFileName();
    QString getFilePath();
    const QString& getComment();
    const QString& getVersion();


private:
    QMap<QString, int> specs;
    QFileInfo fileInfo;
    QString comment;
    QString version;
    bool valid;

    void init();
    static QMap<QString, int> mapSpecs(const QString& str);
    static int letterCount(const QString& str, int pos);
    static int numberCount(const QString& str, int pos);
};

#endif // BITSTREAMSPECS_H
