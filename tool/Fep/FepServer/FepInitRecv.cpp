#include "FepInitRecv.h"
#include "FepFileOperate.h"
#include "Protocol.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

FepInitRecv::FepInitRecv(RecvContent* content, QObject* parent)
    : IStateRecv(content, parent)
{
}

FepInitRecv::~FepInitRecv() {}

void FepInitRecv::sendRequest() {}

#define REMOVE_FILE(x)                                                                                                                               \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (!QFile::remove(x))                                                                                                                       \
        {                                                                                                                                            \
            emit sg_stateChange(RecvState::Fault, "本地文件已存在,但是文件无法操作");                                                                \
            return;                                                                                                                                  \
        }                                                                                                                                            \
    } while (false)

void FepInitRecv::fileExistsOpt(QFileInfo& info, const REQPack& pack)
{
    /* 认为是断点续传 */
    auto curFileSize = info.size();

    qint32 unitNum = 0;
    /* 未传完的文件居然比原文件大肯定不是同一个文件,删除重来 */
    if (curFileSize > pack.fileLen)
    {
        REMOVE_FILE(m_content->curFileInfo.absoluteFilePath);
        qDebug() << "文件错误";
    }

    /*
     * 相等认为是同一个文件
     * 这里根据协议没有做很严格的判断,直接比对文件大小相等就认为是同一个文件了
     * 协议中只能根据文件名与文件大小来做模糊判断
     */
    else if (curFileSize == pack.fileLen)
    {
        qDebug() << "文件已存在,接收下一个文件";
        /* 文件已经存在 */
        unitNum = -1;
    }
    else
    {
        qDebug() << "断点续传";
        /* 不是发送单元的整倍数,认为文件错误删掉重传 */
        if (curFileSize % DefLen::dataMaxLen != 0)
        {
            REMOVE_FILE(m_content->curFileInfo.absoluteFilePath);
        }

        /* 条件符合了,认为是同一个文件,开始计算单元号 */
        unitNum = curFileSize / DefLen::dataMaxLen;
    }

    AnserPack anserPack;
    anserPack.fileID = FepFileOperate::createFileID(m_content->curFileInfo.absoluteFilePath);
    anserPack.dataUnitNum = unitNum;
    anserPack.fileName = pack.fileName;

    m_content->curUnitNum = unitNum;
    m_content->recvFileSize = static_cast<quint32>(unitNum) * static_cast<quint32>(DefLen::dataMaxLen);

    QByteArray sendByte;
    QString errMsg;
    if (!anserPack.toByteArray(sendByte, errMsg))
    {
        emit sg_stateChange(RecvState::Abort, errMsg);
        return;
    }
    /* 完整的文件,下一个 */
    if (unitNum < 0)
    {
        emit sg_stateChange(RecvState::FullFile);
    }

    emit sg_sendData(sendByte);
}
void FepInitRecv::recvResponse(const QByteArray& rawData)
{
    REQPack pack;
    QString errMsg;
    if (!pack.fromByteArray(rawData, errMsg))
    {
        emit sg_stateChange(RecvState::Abort, errMsg);
        return;
    }
    if (pack.fileLen == 0 || pack.fileName.isEmpty())
    {
        emit sg_stateChange(RecvState::Abort, "传输文件错误无文件名或无大小");
        return;
    }
    qDebug() << "接收文件:" << pack.fileName << "文件大小:" << pack.fileLen;
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
