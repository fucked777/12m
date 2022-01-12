#include "FepHelper.h"
#include "Protocol.h"

RecvState FepHelper::recvSendREQ(QString& errMsg, const QByteArray& rawData)
{
    REQPack pack;
    if (!pack.fromByteArray(rawData, errMsg))
    {
        return RecvState::Abort;
    }
    if (pack.fileLen == 0 || pack.fileName.isEmpty())
    {
        errMsg = "传输文件错误无文件名或无大小";
        return RecvState::Abort;
    }

    m_fileName = pack.fileName;
    m_content->curFileInfo.fileName = pack.fileName;
    m_content->curFileInfo.fileLength = pack.fileLen;

    QDir dir(m_content->workDir);
    m_content->curFileInfo.absoluteFilePath = dir.filePath(pack.fileName);

    QFileInfo info(m_content->curFileInfo.absoluteFilePath);

    /*
     * 文件不存在直接发送就ok
     * 文件存在
     *  文件比客户端发送的文件小且文件大小能被最大单次数据大小整除 断点重传
     *  文件比客户端发送的文件小且文件大小能被不能最大单次数据大小整除 删除重传
     */
    if (info.exists())
    {
        fileExistsOpt(info, pack);
        return;
    }

    /* 文件不存在 */
    AnserPack anserPack;
    anserPack.fileID = FepFileOperate::createFileID(m_content->curFileInfo.absoluteFilePath);
    anserPack.dataUnitNum = 0;
    anserPack.fileName = pack.fileName;
    m_content->curUnitNum = 0;
    m_content->recvFileSize = 0;

    QByteArray sendByte;
    if (!anserPack.toByteArray(sendByte, errMsg))
    {
        emit sg_stateChange(RecvState::Abort, errMsg);
        return;
    }
    emit sg_sendData(sendByte);
}
