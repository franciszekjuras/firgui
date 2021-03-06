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

    bool isValid() const;
    const QMap<QString, int>& getSpecs() const;
    QString getFileName() const;
    QString getFilePath() const;
    const QString& getComment() const;
    int getMajVersion() const;
    int getSubVersion() const;


private:
    QMap<QString, int> specs;
    QFileInfo fileInfo;
    QString comment;
    int majVersion;
    int subVersion;
    bool valid;

    void init();
    static QMap<QString, int> mapSpecs(const QString& str);
    bool setVersion(const QString& str);
    static int letterCount(const QString& str, int pos);
    static int numberCount(const QString& str, int pos);
};

#endif // BITSTREAMSPECS_H
