#include "INetMsg.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "RecvMsgPoll.h"
#include "SocketPackHelper.h"
#include "UserManagerMessageSerialize.h"
#include "Utility.h"
#include "ZMQUtility.h"
#include <QByteArray>
#include <QDebug>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QString>
#include <QThread>
#include <QUuid>
#include <QtConcurrent>
#include <functional>

struct SubscribeFuncInfo
{
    explicit SubscribeFuncInfo(const std::function<INetMsg::SubscribeFunc>& func_, bool localThread_)
        : func(func_)
        , localThread(localThread_)
    {
    }
    SubscribeFuncInfo() {}
    std::function<INetMsg::SubscribeFunc> func;
    bool localThread{ true };
};

class INetMsgImpl
{
    DIS_COPY_MOVE(INetMsgImpl)
public:
    INetMsg* self;
    /* 订阅的函数 */
    QHash<QString, SubscribeFuncInfo> subscribe;

    QString sendID;
    QString recvID;
    QString srcID;
    QString addr;
    QString threadName;

    /* 锁 */
    QMutex mutex;
    /* 发送的Socket */
    ZMQSocket zmqSendSocket;

public:
    explicit inline INetMsgImpl(INetMsg* netMsg)
        : self(netMsg)
    {
    }
    inline ~INetMsgImpl() {}
};

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
INetMsg::INetMsg(QObject* parent)
    : INetMsg(QString(), parent)
{
}
INetMsg::INetMsg(const QString& threadName, QObject* parent)
    : QObject(parent)
    , m_baseImpl(new INetMsgImpl(this))
{
    qRegisterMetaType<SubscribeFunction>("SubscribeFunction");
    qRegisterMetaType<SubscribeFunction>("const SubscribeFunction&");
    qRegisterMetaType<ProtocolPack>("ProtocolPack");
    qRegisterMetaType<ProtocolPack>("const ProtocolPack&");

    m_baseImpl->threadName = (threadName.isEmpty() ? "RecvMsgPoll" : threadName);

    connect(this, &INetMsg::sg_recvMsg, this, &INetMsg::recvMsg);
}
INetMsg::~INetMsg()
{
    stopServer();
    delete m_baseImpl;
}

void INetMsg::setSrcAddr(const QString& id)
{
    if (id.isEmpty())
    {
        return;
    }
    /*
     * 20210502 这里将当前软件的instance id加上去 否则在多客户端时会产生冲突的未定义行为
     *
     */
    m_baseImpl->sendID = QString("%1_%2%3").arg(PlatformConfigTools::instanceID(), id, SocketPackHelper::getSendIDSuffix());
    m_baseImpl->recvID = QString("%1_%2%3").arg(PlatformConfigTools::instanceID(), id, SocketPackHelper::getRecvIDSuffix());
    m_baseImpl->srcID = id;
}
QString INetMsg::sendAddr() const { return m_baseImpl->sendID; }
QString INetMsg::recvAddr() const { return m_baseImpl->recvID; }
/*
 * 注册订阅的函数
 * key  为需要订阅的类型与ProtocolPack::operation  内容一致就会触发调用
 * func 为收到消息后触发的函数
 * 一个key只能对应一个触发函数,多次注册只会保留最后一次
 */
void INetMsg::registerSubscribe(const QString& key, const std::function<INetMsg::SubscribeFunc>& func, bool localThread)
{
    if (key.isEmpty() || func == nullptr)
    {
        return;
    }
    (m_baseImpl->subscribe)[key] = SubscribeFuncInfo(func, localThread);
}
void INetMsg::unregisterSubscribe(const QString& key) { m_baseImpl->subscribe.remove(key); }
void INetMsg::setConnectAddr(const QString& addr) { m_baseImpl->addr = addr; }
QString INetMsg::connectAddr() const { return m_baseImpl->addr; }
bool INetMsg::startServer(QString* errMsg)
{
    m_baseImpl->zmqSendSocket.destroy();

    auto zmqContextResult = SocketPackHelper::registerZMQContext();
    if (!zmqContextResult)
    {
        Utility::nullAssignment(errMsg, zmqContextResult.msg());
        return false;
    }
    auto zmqContext = zmqContextResult.value();

    auto initRes = SocketPackHelper::initDealerSocket(*zmqContext, m_baseImpl->zmqSendSocket, m_baseImpl->addr, m_baseImpl->sendID);
    if (!initRes)
    {
        auto msg = QString("初始化ZMQ发送Socket错误:%1 当前SrcID:%2").arg(initRes.msg(), m_baseImpl->srcID);
        Utility::nullAssignment(errMsg, msg);
        return false;
    }

    /* 获取平台对象 */
    auto platformObjectTools = PlatformObjectTools::instance();
    auto recvMsgPollObj = platformObjectTools->addObj<RecvMsgPoll>(m_baseImpl->threadName);
    if (recvMsgPollObj == nullptr)
    {
        auto msg = QString("注册ZMQ接收Socket错误:%1 当前SrcID:%2").arg(initRes.msg(), m_baseImpl->srcID);
        Utility::nullAssignment(errMsg, msg);
        return false;
    }
    /* 获取 */
    auto registerRes = recvMsgPollObj->registerPollItem(m_baseImpl->recvID, m_baseImpl->addr, this);
    if (!registerRes)
    {
        Utility::nullAssignment(errMsg, registerRes.msg());
        return false;
    }

    return true;
}

void INetMsg::stopServer()
{
    m_baseImpl->zmqSendSocket.destroy();

    /* 获取平台对象 */
    auto platformObjectTools = PlatformObjectTools::instance();
    auto recvMsgPollObj = platformObjectTools->getObj<RecvMsgPoll>(m_baseImpl->threadName);
    if (recvMsgPollObj != nullptr)
    {
        recvMsgPollObj->removePollItem(this);
    }
}
OptionalNotValue INetMsg::send(const ProtocolPack& recvValue)
{
    auto bak = recvValue;
    auto currentuser = GlobalData::getCurrentUser();
    bak.userID = currentuser.name;
    if (bak.userID.isEmpty())
    {
        bak.userID = "Unknown";
    }

    return sendManual(bak);
}
OptionalNotValue INetMsg::sendManual(const ProtocolPack& recvValue)
{
    if (!m_baseImpl->zmqSendSocket.valid())
    {
        return OptionalNotValue(ErrorCode::NotInit, "发送通道未初始化");
    }

    QMutexLocker locker(&(m_baseImpl->mutex));

    QString errMsg;
    auto bak = recvValue;

    /* 目标选项为空,不发送 */
    if (bak.operation.isEmpty())
    {
        return OptionalNotValue();
    }

    /*
     * 对内发送 是 当前实例ID_接收地址_接收后缀
     * 对外发送 是 对方实例ID_接收地址_接收后缀
     */
    bak.srcID = m_baseImpl->sendID;
    bak.desID = QString("%1_%2%3").arg(recvValue.module ? PlatformConfigTools::instanceID() : ExtendedConfig::communicationInstanceID(), bak.desID,
                                       SocketPackHelper::getRecvIDSuffix());

    //    if (bak.desID.endsWith(SocketPackHelper::getRecvIDSuffix()))
    //    {
    //        bak.desID = QString("%1_%2").arg(recvValue.module ? PlatformConfigTools::instanceID() : ExtendedConfig::communicationInstanceID(),
    //        bak.desID);
    //    }
    //    else
    //    {
    //        bak.desID = QString("%1_%2%3").arg(recvValue.module ? PlatformConfigTools::instanceID() : ExtendedConfig::communicationInstanceID(),
    //                                           bak.desID, SocketPackHelper::getRecvIDSuffix());
    //    }

    auto res = SocketPackHelper::dealerSend(m_baseImpl->zmqSendSocket, bak, errMsg, ZMQ_DONTWAIT);
    return res.success() ? OptionalNotValue() : OptionalNotValue(ErrorCode::InitFailed, errMsg);
}
OptionalNotValue INetMsg::sendACK(const ProtocolPack& recvValue)
{
    if (!m_baseImpl->zmqSendSocket.valid())
    {
        return OptionalNotValue(ErrorCode::NotInit, "发送通道未初始化");
    }

    QMutexLocker locker(&(m_baseImpl->mutex));

    QString errMsg;
    auto bak = recvValue;
    std::swap(bak.operation, bak.operationACK);
    /* 目标选项为空,不发送 */
    if (bak.operation.isEmpty())
    {
        return OptionalNotValue();
    }
    /* 交换源目的地址 */
    /* 这里目标地址要换一下,要把发送改为接收 */
    bak.desID = SocketPackHelper::removeSendIDSuffix(bak.srcID);
    bak.desID = QString("%1%2").arg(bak.desID, SocketPackHelper::getRecvIDSuffix());
    bak.srcID = m_baseImpl->sendID;

    auto res = SocketPackHelper::dealerSend(m_baseImpl->zmqSendSocket, bak, errMsg, ZMQ_DONTWAIT);

    return res.success() ? OptionalNotValue() : OptionalNotValue(ErrorCode::DataSendFailed, errMsg);
}
void INetMsg::recvMsg(const ProtocolPack& pack)
{
    /* 没找到 子选项 */
    auto find = m_baseImpl->subscribe.find(pack.operation);
    if (find == m_baseImpl->subscribe.end())
    {
        return;
    }
    auto& funcInfo = find.value();
    if (funcInfo.localThread)
    {
        funcInfo.func(pack);
    }
    else
    {
        QtConcurrent::run([=]() { (funcInfo.func)(pack); });
    }
}
