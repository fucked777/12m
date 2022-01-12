#include "RecvMsgPoll.h"
#include "INetMsg.h"
#include "MacroHelper.h"
#include "SocketPackHelper.h"
#include "ZMQContext.h"
#include "ZMQHelper.h"
#include "ZMQSocket.h"
#include <QApplication>
#include <QCoreApplication>
#include <QMutex>
#include <QScopedArrayPointer>

/* 内部线程运行的临时结构体 */
struct TempInternalPoll
{
    ZMQSocket* socket;
    INetMsg* msg;
    zmq_pollitem_t* pollitem;
    TempInternalPoll()
        : socket(nullptr)
        , msg(nullptr)
        , pollitem(nullptr)
    {
    }
    TempInternalPoll(ZMQSocket* socket_, INetMsg* msg_, zmq_pollitem_t* pollitem_)
        : socket(socket_)
        , msg(msg_)
        , pollitem(pollitem_)
    {
    }
};
struct ZMQMsgSocketInfo
{
    QString recvID;
    QString addr;
    INetMsg* netMsg{ nullptr };
};

class RecvMsgPollImpl
{
public:
    QMap<INetMsg*, ZMQMsgSocketInfo> msgSocketInfoMap;
    /* 运行标志 true代表运行 */
    std::atomic_bool runningFlag{ false };
};

RecvMsgPoll::RecvMsgPoll()
    : m_impl(new RecvMsgPollImpl)
{
}
RecvMsgPoll::~RecvMsgPoll()
{
    stopServer();
    delete m_impl;
}

OptionalNotValue RecvMsgPoll::registerPollItem(QString recvID, QString addr, INetMsg* netMsg)
{
    if (netMsg == nullptr)
    {
        return OptionalNotValue(ErrorCode::InvalidArgument, "参数不能为nullptr");
    }

    stopServer();

    ZMQMsgSocketInfo tempInfo;
    tempInfo.recvID = recvID;
    tempInfo.addr = addr;
    tempInfo.netMsg = netMsg;
    m_impl->msgSocketInfoMap.insert(netMsg, tempInfo);

    startServer();
    return OptionalNotValue();
}
void RecvMsgPoll::removePollItem(INetMsg* netMsg)
{
    auto find = m_impl->msgSocketInfoMap.find(netMsg);
    if (find != m_impl->msgSocketInfoMap.end())
    {
        stopServer();

        m_impl->msgSocketInfoMap.erase(find);

        startServer();
    }
}
void RecvMsgPoll::stopServer()
{
    m_impl->runningFlag.store(false);
    quit();

    while (isRunning() && !isFinished())
    {
        QThread::msleep(10);
        QApplication::processEvents();
    }

    wait();
}

void RecvMsgPoll::startServer()
{
    if (!isRunning())
    {
        m_impl->runningFlag.store(true);
        start();
    }
}
void RecvMsgPoll::run()
{
    if (m_impl->msgSocketInfoMap.isEmpty())
    {
        return;
    }
    auto destructionObjFunc = [](QMap<INetMsg*, ZMQSocket*>& msg) {
        for (auto& item : msg)
        {
            delete item;
        }
        msg.clear();
    };

    auto zmqContextResult = SocketPackHelper::registerZMQContext();
    if (!zmqContextResult)
    {
        qWarning() << QString("RecvMsgPoll Error:%1").arg(zmqContextResult.msg());
        return;
    }
    auto zmqContext = zmqContextResult.value();

    ProtocolPack pack;
    QString errMsg;
    ZMQSocketRes zmqSocketRes;
    auto size = m_impl->msgSocketInfoMap.size();
    QMap<INetMsg*, ZMQSocket*> msgMap;
    QScopedArrayPointer<zmq_pollitem_t> pollItems(new zmq_pollitem_t[size]);
    QScopedArrayPointer<TempInternalPoll> tempInternalPol(new TempInternalPoll[size]);

    int index = 0;
    for (auto& item : m_impl->msgSocketInfoMap)
    {
        if (!m_impl->runningFlag)
        {
            destructionObjFunc(msgMap);
            return;
        }
        auto tempSocket = new ZMQSocket;
        GetValueHelper<ZMQSocket> getValueHelper(tempSocket);
        auto initRes = SocketPackHelper::initDealerSocket(*zmqContext, *getValueHelper, item.addr, item.recvID);
        if (!initRes)
        {
            qWarning() << QString("RecvMsgPoll Error:%1 addr:%2 recvID:%3").arg(initRes.msg(), item.addr, item.recvID);
            continue;
        }

        auto& temp = tempInternalPol[index];
        pollItems[index] = { tempSocket->handle(), 0, ZMQ_POLLIN, 0 };
        temp.pollitem = &(pollItems[index]);
        temp.msg = item.netMsg;
        temp.socket = tempSocket;

        msgMap.insert(item.netMsg, getValueHelper.get());
        ++index;
    }
    if (index <= 0)
    {
        destructionObjFunc(msgMap);
        return;
    }

    while (m_impl->runningFlag && !QCoreApplication::closingDown())
    {
        auto rc = zmq_poll(pollItems.data(), index, SocketPackHelper::getTimeo());
        // 没有消息
        if (rc == 0)
        {
            continue;
        }
        /* 发生错误 */
        else if (rc <= 0)
        {
            qWarning() << QString("ZMQPollServer Error:%1").arg(ZMQHelper::errMsg());
            break;
        }

        for (int i = 0; i < size; ++i)
        {
            auto& temp = tempInternalPol[i];
            if (temp.pollitem->revents & ZMQ_POLLIN)
            {
                /* 收数据 */
                zmqSocketRes = SocketPackHelper::dealerRecv(*(temp.socket), pack, errMsg);
                /* 没收到任何消息 */
                if (zmqSocketRes.eagain())
                {
                    break;
                }
                /* 产生了错误 */
                else if (!zmqSocketRes.success())
                {
                    qWarning() << QString("ZMQPollServer Recv Data Error:%1").arg(errMsg);
                    break;
                }

                emit temp.msg->sg_recvMsg(pack);
                break;
            }
        }
    }

    destructionObjFunc(msgMap);
}
