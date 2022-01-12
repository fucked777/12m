#include "FepFileOperate.h"
#include <QFileInfo>
#include <cstdlib>

bool FepFileOperate::getSendFileList(SendContent& content, QString& errMsg)
{
    content.fileList.clear();
    if (content.workDir.isEmpty())
    {
        errMsg = "目录路径为空";
        return false;
    }
    QDir dir(content.workDir);
    if (!dir.exists())
    {
        errMsg = "当前目录不在";
        return false;
    }
    content.workDir = dir.absolutePath();
    auto fileList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
    for (auto& file : fileList)
    {
        SendFileInfo tmpFileInfo;
        tmpFileInfo.fileName = file.fileName();
        tmpFileInfo.absoluteFilePath = file.absoluteFilePath();
        tmpFileInfo.length = file.size();
        // tmpFileInfo.sendUnitNum = 0;
        // tmpFileInfo.fileId = 0;
        // tmpFileInfo.haveSendLength = 0;
        // tmpFileInfo.IsSendFinish = false;
        content.fileList.append(tmpFileInfo);
    }
    content.curFileInfo = content.fileList.isEmpty() ? nullptr : &(content.fileList[0]);

    return true;
}
void FepFileOperate::nextFile(SendContent& content)
{
    /* 这里不应该出现直接就是空的情况 */
    Q_ASSERT(!content.fileList.isEmpty());
    //    if (m_fileList.isEmpty())
    //    {
    //        m_curFileInfo = nullptr;
    //        return;
    //    }
    content.fileList.pop_front();
    if (content.fileList.isEmpty())
    {
        content.curFileInfo = nullptr;
        return;
    }
    content.curFileInfo = &(content.fileList.first());
}
bool FepFileOperate::readFileContent(SendContent& content, QByteArray& fileContent, quint32& length, QString& errMsg)
{
    if (content.fileList.isEmpty())
    {
        errMsg = "待发送文件列表为空";
        return false;
    }
    QFile tmpFile(content.curFileInfo->absoluteFilePath);
    bool ret = tmpFile.open(QIODevice::ReadOnly);
    if (!ret)
    {
        errMsg = QString("文件不存在:%1").arg(content.curFileInfo->absoluteFilePath);
        return false;
    }

    length = 0;
    fileContent.clear();

    if (content.curFileInfo->sendLength < content.curFileInfo->length)
    {
        tmpFile.seek(content.curFileInfo->sendLength);
        fileContent = tmpFile.read(DefLen::dataMaxLen);
        length = fileContent.size();
        content.curFileInfo->sendLength += length;
    }

    ++content.curFileInfo->curUnitNum;
    return true;
}
bool FepFileOperate::setFile(SendContent& content, const QString& filePath, QString& errMsg)
{
    content.fileList.clear();

    QFileInfo info(filePath);
    if (!info.isFile())
    {
        errMsg = "文件不存在";
        return false;
    }
    SendFileInfo tmpFileInfo;
    tmpFileInfo.fileName = info.fileName();
    tmpFileInfo.absoluteFilePath = info.absoluteFilePath();
    tmpFileInfo.length = info.size();
    // tmpFileInfo.sendUnitNum = 0;
    // tmpFileInfo.fileId = 0;
    // tmpFileInfo.haveSendLength = 0;
    // tmpFileInfo.IsSendFinish = false;
    content.fileList.append(tmpFileInfo);
    auto dir = info.absoluteDir();
    content.workDir = dir.absolutePath();

    content.curFileInfo = &(content.fileList[0]);
    return true;
}
//获取发送文件
bool FepFileOperate::getTheSendFile(SendContent& content, SendFileInfo& needSendFile)
{
    if (content.fileList.isEmpty())
    {
        return false;
    }

    needSendFile = *(content.curFileInfo);
    return true;
}
bool FepFileOperate::getTheSendFileP(SendContent& content, SendFileInfo*& needSendFile)
{
    if (content.fileList.isEmpty())
    {
        return false;
    }

    needSendFile = content.curFileInfo;
    return true;
}
quint16 FepFileOperate::createFileID(const QString& /*fileName*/) { return static_cast<quint16>(std::rand()); }
