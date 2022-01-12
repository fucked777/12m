#include "FepRecvData.h"
#include "FepFileOperate.h"
#include "Protocol.h"

FepRecvData::FepRecvData(RecvContent* content, QObject* parent)
    : IStateRecv(content, parent)
{
}
FepRecvData::~FepRecvData() {}
//发送数据--发送的时候需要人为暂停否
void FepRecvData::sendRequest()
{
    FinishPack pack;
    pack.fileID = m_content->curFileInfo.fileID;

    QByteArray sendData;
    QString errMsg;
    if (!pack.toByteArray(sendData, errMsg))
    {
        emit sg_stateChange(RecvState::Abort, errMsg);
        return;
    }
    emit sg_sendData(sendData);
}
void FepRecvData::recvResponse(const DataPack& pack)
{
    QString errMsg;
    /* 文件总大小 */
    auto fileLength = m_content->curFileInfo.fileLength;
    /* 当前已接收文件的大小 */
    auto& recvLen = m_content->recvFileSize;
    /* 当前应该接收的数据包大小 */
    auto curRecvDataLen = fileLength > recvLen + DefLen::dataMaxLen ? DefLen::dataMaxLen : fileLength - recvLen;

    /* 每次的单云号应当递增1 如果不是则数据包错误 */
    if (m_content->curUnitNum != pack.dataUnitNum)
    {
        emit sg_stateChange(RecvState::Abort, "数据包错误,当前单元号与预期不符");
        return;
    }
    /*
     * 根据协议当文件大小为单元大小的整倍数时会在发完数据后多发送一次字节为为0的数据包以示终止
     * 这个就是最后一次
     */
    if (static_cast<quint32>(recvLen) == fileLength)
    {
        /* 等待下一个文件 */
        emit sg_stateChange(RecvState::Wait);
        return;
    }
    QByteArray tempData(pack.data.data(), curRecvDataLen);

    if (!FepFileOperate::writeToFile(*m_content, tempData, errMsg))
    {
        emit sg_stateChange(RecvState::Abort, errMsg);
        return;
    }

    ++m_content->curUnitNum;
    recvLen += curRecvDataLen;

    /* 认为接收完了  */
    if (recvLen == fileLength)
    {
        emit sg_stateChange(RecvState::Wait);
        return;
    }
}

void FepRecvData::recvResponse(const QByteArray& /*rawData*/) {}
