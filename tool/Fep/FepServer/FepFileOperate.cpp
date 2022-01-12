#include "FepFileOperate.h"
#include "Protocol.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <cstdlib>

//读取文件夹里面的文件
bool FepFileOperate::createWorkDir(QString& workDir, QString& errMsg)
{
    if (workDir.isEmpty())
    {
        errMsg = "目录路径为空";
        return false;
    }
    QDir dir;
    if (!dir.exists(workDir))
    {
        if (!dir.mkpath(workDir))
        {
            errMsg = "目录创建失败";
            return false;
        }
    }
    dir.setCurrent(workDir);
    workDir = dir.absolutePath();
    return true;
}
bool FepFileOperate::writeToFile(RecvContent& content, const QByteArray& writeData, QString& errMsg)
{
    if (!content.file.isOpen())
    {
        content.file.setFileName(content.curFileInfo.absoluteFilePath);
        if (!content.file.open(QFile::WriteOnly | QFile::Append))
        {
            errMsg = QString("%1:%2").arg("文件打开失败", content.file.errorString());
            return false;
        }
    }

    auto size = content.file.write(writeData);
    if (size < 0 || size != writeData.size())
    {
        errMsg = QString("%1:%2").arg("数据写入失败", content.file.errorString());
        content.file.flush();
        return false;
    }

    return true;
}

quint16 FepFileOperate::createFileID(const QString& /*fileName*/) { return static_cast<quint16>(std::rand()); }
