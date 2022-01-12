#ifndef FEPFILEOPERATE_H
#define FEPFILEOPERATE_H

#include "Protocol.h"
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QObject>

class FepFileOperate : public QObject
{
    Q_OBJECT
public:
    static bool getSendFileList(SendContent&, QString& errMsg);
    static void nextFile(SendContent&);
    static bool readFileContent(SendContent&, QByteArray& fileContent, quint32& length, QString& errMsg);
    static bool setFile(SendContent&, const QString& filePath, QString& errMsg);
    static bool getTheSendFile(SendContent&, SendFileInfo& needSendFile);
    static bool getTheSendFileP(SendContent&, SendFileInfo*& needSendFile);
    static quint16 createFileID(const QString& /*fileName*/);
};

#endif  // CFEPFILEOPERATE_H
