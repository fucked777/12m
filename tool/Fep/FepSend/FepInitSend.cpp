#include "FepInitSend.h"
#include "FepFileOperate.h"

FepInitSend::FepInitSend(SendContent* content, QObject* parent)
    : IStateSend(content, parent)
{
}

FepInitSend::~FepInitSend() {}

void FepInitSend::sendRequest()
{
    SendFileInfo currFile;
    if (!FepFileOperate::getTheSendFile(*m_content, currFile))
    {
        qDebug() << "发送完成";
        //退出整个系统，因为没有文件可以发送
        emit sg_stateChange(SendState::Close);
        return;
    }

    REQPack pack;
    pack.fileName = currFile.fileName;
    pack.fileLen = currFile.length;

    QByteArray sendata;
    QString errMsg;
    if (!pack.toByteArray(sendata, errMsg))
    {
        emit sg_stateChange(SendState::Abort, errMsg);
        return;
    }

    emit sg_sendData(sendata);
}

void FepInitSend::recvResponse(const QByteArray& rawData)
{
    AnserPack pack;
    QString errMsg;
    if (!pack.fromByteArray(rawData, errMsg))
    {
        emit sg_stateChange(SendState::Abort, errMsg);
        return;
    }
    /* 这里文件不可能为空的,为空就是GG了 */
    SendFileInfo* currFile = nullptr;
    if (!FepFileOperate::getTheSendFileP(*m_content, currFile))
    {
        Q_ASSERT(false);
        emit sg_stateChange(SendState::Abort, "系统错误,获取文件失败");
        return;
    }

    /* 解析成功 */
    currFile->curUnitNum = pack.dataUnitNum;
    currFile->sendLength = 0;
    currFile->fileID = pack.fileID;

    if (pack.dataUnitNum >= 0)
    {
        currFile->sendLength = currFile->curUnitNum * DefLen::dataMaxLen;
        if (currFile->sendLength > currFile->length)
        {
            auto sendStr = QString("数据错误,本地文件大小%1,远端断点续传文件大小%2").arg(currFile->length, currFile->sendLength);
            emit sg_stateChange(SendState::Abort, sendStr);
            return;
        }
        /* 文件不存在，可以进行传输 */
        /* 断点续传 */
        emit sg_stateChange(SendState::SendFile);
        return;
    }
    else if (pack.dataUnitNum == -1)
    {
        /* 文件是完整的,下一个文件 */
        emit sg_stateChange(SendState::Next);
        return;
    }
    else if (pack.dataUnitNum == -2)
    {
        /* 暂时故障不能接收文件 */
        emit sg_stateChange(SendState::Close, "服务端故障,不能接收文件");
        return;
    }

    emit sg_stateChange(SendState::Abort, "接收数据错误,未知的信息");
}
