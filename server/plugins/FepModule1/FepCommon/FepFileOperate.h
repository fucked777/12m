#ifndef FEPFILEOPERATE_H
#define FEPFILEOPERATE_H

#include "Protocol.h"
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QString>

struct RecvContent;
class FepFileOperate
{
public:
    //设置需要读取的文件夹
    static bool createWorkDir(QString& workDir, QString& errMsg);
    static bool writeToFile(RecvContent&, const QByteArray&, QString& errMsg);
    static quint16 createFileID(const QString& /*fileName*/);

    static bool getSendFileList(SendContent&, QString& errMsg);
    static void nextFile(SendContent&);
    static bool readFileContent(SendContent&, QByteArray& fileContent, quint32& length, QString& errMsg);
    static bool setFile(SendContent&, const QString& filePath, QString& errMsg);
    static bool getTheSendFile(SendContent&, SendFileInfo& needSendFile);
    static bool getTheSendFileP(SendContent&, SendFileInfo*& needSendFile);
};

#endif  // FEPFILEOPERATE_H
