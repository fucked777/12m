#include "FepSendData.h"
#include "FepFileOperate.h"
#include <QApplication>
#include <QThread>

FepSendData::FepSendData(SendContent* content, QObject* parent)
    : IStateSend(content, parent)
{
}
FepSendData::~FepSendData() {}
//发送数据--发送的时候需要人为暂停否
void FepSendData::sendRequest()
{
    /* 这里文件不可能为空的,为空就是GG了 */
    SendFileInfo* currFile = nullptr;
    if (!FepFileOperate::getTheSendFileP(*m_content, currFile))
    {
        Q_ASSERT(false);
        emit sg_stateChange(SendState::Abort, "系统错误,获取文件失败");
        return;
    }

    DataPack pack;
    //此处根据文件大小来做
    while (m_content->state == SendState::SendFile)
    {
        pack.dataUnitNum = currFile->curUnitNum;
        pack.fileID = currFile->fileID;

        pack.data.clear();
        quint32 length = 0;
        QString errMsg;
        auto ret = FepFileOperate::readFileContent(*m_content, pack.data, length, errMsg);
        if (!ret)
        {
            emit sg_stateChange(SendState::Abort, errMsg);
            break;
        }

        QByteArray sendData;
        if (!pack.toByteArray(sendData, errMsg))
        {
            emit sg_stateChange(SendState::Abort, errMsg);
            return;
        }
        emit sg_sendData(sendData);
        /* 延时一下 等待接收完 */
        QThread::msleep(10);

        if (currFile->sendLength >= currFile->length)
        {
            /* 发完了 */
            /* 这种情况要发送一个0字节 */
            if (static_cast<int>(length) == DefLen::dataMaxLen)
            {
                pack.data = QByteArray();
                pack.dataUnitNum = currFile->curUnitNum;
                ++currFile->curUnitNum;
                pack.toByteArray(sendData, errMsg);
                emit sg_sendData(sendData);
            }

            // 发送完成等待回复
            // emit sg_stateChange(SendState::Next);
            break;
        }
        emit sg_timeReset();
        QApplication::processEvents();
    }
}

void FepSendData::recvResponse(const QByteArray& /*rawData*/) {}
