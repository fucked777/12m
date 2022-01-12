#ifndef CFEPFILEOPERATE_H
#define CFEPFILEOPERATE_H

#include <QString>
struct RecvContent;
class FepFileOperate
{
public:
    //设置需要读取的文件夹
    static bool createWorkDir(QString& workDir, QString& errMsg);
    static bool writeToFile(RecvContent&, const QByteArray&, QString& errMsg);
    static quint16 createFileID(const QString& /*fileName*/);
};

#endif  // CFEPFILEOPERATE_H
