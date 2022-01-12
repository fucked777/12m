#include "INetMsg.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
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

/* 放在类里面得声明要写两个 即使是constexpr  不然麒麟会提示缺少符号 艹艹 */
static constexpr int timeo = 1000;
static constexpr int linger = 1000;
static constexpr int sendHWM = 1000;
static constexpr int recvHWM = 1000;
static constexpr const char sendIDSuffix[] = "_send";
static constexpr const char recvIDSuffix[] = "_recv";

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

class INetMsgImpl : public QThread
{
    DIS_COPY_MOVE(INetMsgImpl)
public:
    INetMsg* self;
    /* 订阅的函数 */
    QHash<QString, SubscribeFuncInfo> subscribe;

    /* 运行标志 true代表运行 */
    std::atomic_bool runningFlag{ false };
    QString sendID;
    QString recvID;
    QString addr;

    /* zmq的上下文 */
    ZMQContext zmqContext;
    /* 锁 */
    QMutex mutex;
    /* 发送的Socket */
    ZMQSocket zmqSendSocket;

public:
    inline INetMsgImpl(INetMsg* netMsg)
        : self(netMsg)
    {
    }
    inline ~INetMsgImpl() {}
    void run();
    static ZMQSocketRes recv(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg);
    static ZMQSocketRes send(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg, int flags = 0);
    static OptionalNotValue initSocket(ZMQContext& zmqContext, ZMQSocket& zmqSocket, const QString& id, const QString& addr);
};

ZMQSocketRes INetMsgImpl::recv(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg)
{
    /* 1 空帧 */
    auto res = zmqSocket.recv(0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 2 目标地址 */
    res = zmqSocket.recv(pack.desID, 0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 3 空帧 */
    res = zmqSocket.recv(0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 4 发送选项 */
    res = zmqSocket.recv(pack.operation, 0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 5 空帧 */
    res = zmqSocket.recv(0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 6 应答选项 */
    res = zmqSocket.recv(pack.operationACK, 0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 7 空帧 */
    res = zmqSocket.recv(0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 8 模块间通讯 */
    QByteArray array;
    res = zmqSocket.recv(array, 0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }
    pack.module = array.isEmpty() ? false : !!array[0];

    /* 9 空帧 */
    res = zmqSocket.recv(0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 10 发送者 */
    res = zmqSocket.recv(pack.userID, 0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 11 空帧 */
    res = zmqSocket.recv(0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 12 数据 */
    res = zmqSocket.recv(pack.data, 0);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    return res;
}

ZMQSocketRes INetMsgImpl::send(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg, int flags)
{
    /* 1 空帧 */
    auto res = zmqSocket.send(ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 2 目标地址 */
    res = zmqSocket.send(pack.desID, ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 3 空帧 */
    res = zmqSocket.send(ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 4 发送选项 */
    res = zmqSocket.send(pack.operation, ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 5 空帧 */
    res = zmqSocket.send(ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 6 应答选项 */
    res = zmqSocket.send(pack.operationACK, ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 7 空帧 */
    res = zmqSocket.send(ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 8 模块间通讯 */
    QByteArray array(1, static_cast<char>(pack.module));
    res = zmqSocket.send(array, ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 9 空帧 */
    res = zmqSocket.send(ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 10 发送者 */
    res = zmqSocket.send(pack.userID, ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 11 空帧 */
    res = zmqSocket.send(ZMQ_SNDMORE);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    /* 12 数据 */
    res = zmqSocket.send(pack.data, flags);
    if (!res.success())
    {
        errMsg = zmqSocket.getErrMsg();
        return res;
    }

    return res;
}

OptionalNotValue INetMsgImpl::initSocket(ZMQContext& zmqContext, ZMQSocket& zmqSocket, const QString& id, const QString& addr)
{
    if (!zmqSocket.init(zmqContext, ZMQ_DEALER))
    {
        return OptionalNotValue(ErrorCode::ZMQSocketInitFailed, zmqSocket.getErrMsg());
    }

    /* recv超时时间,单位毫秒 */
    zmqSocket.set(ZMQ_RCVTIMEO, timeo);
    /* send超时时间,单位毫秒 */
    zmqSocket.set(ZMQ_SNDTIMEO, timeo);
    /*
     * 默认属性值是0，表示会静默的丢弃不能路由到的消息。属性值是1时，
     * 如果消息不能路由，会返回一个EHOSTUNREACH错误代码
     * 此参数 只是ZMQ_ROUTER 有效
     */
    // zmqSocket.set(ZMQ_ROUTER_MANDATORY, 1);
    /*设置调用zmq_disconnect或zmq_close后尚未接收消息的等待时间。该选项将进一步影响zmq_ctx_term的阻塞时间。
     * 设置为-1表示不丢弃任何消息，zmq_ctx_term将会一直阻塞直到消息全部接收；
     * 设置为0表示丢弃消息并立刻返回；设置为正值x表示，x毫秒后将返回，默认是30000毫秒
     */
    zmqSocket.set(ZMQ_LINGER, linger);

    /*ZMQ_SNDHWM属性将会设置socket参数指定的socket对外发送的消息的高水位。
     * 高水位是一个硬限制，它会限制每一个与此socket相连的在内存中排队的未处理的消息数目的最大值。0值代表着没有限制。
     * 如果已经到达了规定的限制，socket就需要进入一种异常的状态，
     * 表现形式因socket类型而异。socket会进行适当的调节，比如阻塞或者丢弃已发送的消息
     */
    zmqSocket.set(ZMQ_SNDHWM, sendHWM);
    /*ZMQ_RCVHWM属性将会设置socket参数指定的socket进入的消息的高水位。
     * 高水位是一个硬限制，它会限制每一个与此socket相连的在内存中排队的未处理的消息数目的最大值。0值代表着没有限制。
     * 如果已经到达了规定的限制，socket就需要进入一种异常的状态，
     * 表现形式因socket类型而异。socket会进行适当的调节，比如阻塞或者丢弃被发送的消息。
     */
    zmqSocket.set(ZMQ_RCVHWM, recvHWM);

    /* 设置ID */
    zmqSocket.set(ZMQ_IDENTITY, id);

    /* 开启TCP保活机制，防止网络连接因长时间无数据而被中断 */
    zmqSocket.set(ZMQ_TCP_KEEPALIVE, 1);

    /* 网络连接空闲1min即发送保活包 */
    zmqSocket.set(ZMQ_TCP_KEEPALIVE_IDLE, 3);

    /* 连接 */
    if (!zmqSocket.connect(addr))
    {
        return OptionalNotValue(ErrorCode::ZMQConnectFailed, zmqSocket.getErrMsg());
    }
    return OptionalNotValue();
}

void INetMsgImpl::run()
{
    /* 退出后flag自动归位 */
    AtomicFlagHelper runFlag(runningFlag, true);

    /* 接收的数据包 */
    ProtocolPack pack;
    QString errMsg;
    ZMQSocketRes res;
    ZMQSocket zmqSocket;

    auto initRes = INetMsgImpl::initSocket(zmqContext, zmqSocket, recvID, addr);
    if (!initRes)
    {
        qWarning() << "INetMsg InitRecvSocket Error:" << initRes.msg();
        return;
    }

    while (runningFlag)
    {
        /* 收数据 */
        res = INetMsgImpl::recv(zmqSocket, pack, errMsg);
        /* 没收到任何消息 */
        if (res.eagain())
        {
            continue;
        }
        /* 产生了错误 */
        else if (!res.success())
        {
            qWarning() << "INetMsg Recv Data Error:" << errMsg;
            continue;
        }

        /* 没找到 子选项 */
        auto find = subscribe.find(pack.operation);
        if (find == subscribe.end())
        {
            continue;
        }
        auto& funcInfo = find.value();
        if (funcInfo.localThread)
        {
            emit self->sg_internalCall(funcInfo.func, pack);
        }
        else
        {
            QtConcurrent::run([=]() { (funcInfo.func)(pack); });
        }
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
INetMsg::INetMsg(QObject* parent)
    : QObject(parent)
    , m_baseImpl(new INetMsgImpl(this))
{
    qRegisterMetaType<SubscribeFunction>("SubscribeFunction");
    qRegisterMetaType<SubscribeFunction>("const SubscribeFunction&");
    qRegisterMetaType<ProtocolPack>("ProtocolPack");
    qRegisterMetaType<ProtocolPack>("const ProtocolPack&");

    connect(this, &INetMsg::sg_internalCall, this, &INetMsg::subscribeCall);
}
INetMsg::~INetMsg()
{
    stopServer();
    delete m_baseImpl;
}

void INetMsg::setSrcAddr(const QString& id)
{
    if (m_baseImpl->runningFlag)
    {
        return;
    }
    if (id.isEmpty())
    {
        return;
    }
    /*
     * 20210502 这里将当前软件的instance id加上去 否则在多客户端时会产生冲突的未定义行为
     *
     */
    m_baseImpl->sendID = QString("%1_%2%3").arg(PlatformConfigTools::instanceID(), id, sendIDSuffix);
    m_baseImpl->recvID = QString("%1_%2%3").arg(PlatformConfigTools::instanceID(), id, recvIDSuffix);
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
void INetMsg::setConnectAddr(const QString& addr)
{
    /* 已经在运行 */
    if (m_baseImpl->runningFlag && addr.isEmpty())
    {
        return;
    }
    m_baseImpl->addr = addr;
}
QString INetMsg::connectAddr() const { return m_baseImpl->addr; }
bool INetMsg::startServer(QString* errMsg)
{
    /* 已经在运行 */
    if (m_baseImpl->runningFlag)
    {
        Utility::nullAssignment(errMsg, "服务已经运行");
        return false;
    }
    m_baseImpl->zmqSendSocket.destroy();
    m_baseImpl->zmqContext.destroy();

    /* 未初始化的情况才去初始化context */
    if (!m_baseImpl->zmqContext.init())
    {
        Utility::nullAssignment(errMsg, m_baseImpl->zmqContext.getErrMsg());
        return false;
    }

    auto initRes = INetMsgImpl::initSocket(m_baseImpl->zmqContext, m_baseImpl->zmqSendSocket, m_baseImpl->sendID, m_baseImpl->addr);
    if (!initRes)
    {
        qWarning() << "INetMsg InitSendSocket Error:" << initRes.msg();
    }

    m_baseImpl->start();
    return true;
}

void INetMsg::stopServer()
{
    if (!m_baseImpl->isRunning())
    {
        return;
    }
    m_baseImpl->runningFlag = false;
    m_baseImpl->quit();
    m_baseImpl->wait();

    m_baseImpl->zmqSendSocket.destroy();
    m_baseImpl->zmqContext.destroy();
}
OptionalNotValue INetMsg::send(const ProtocolPack& recvValue)
{
    auto bak = recvValue;
    User user;
    GlobalData::getCurrentUser(user);
    bak.userID = user.name;
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

    bak.srcID = m_baseImpl->sendID;
    if (bak.desID.endsWith(recvIDSuffix))
    {
        bak.desID = QString("%1_%2").arg(recvValue.module ? PlatformConfigTools::instanceID() : ExtendedConfig::communicationInstanceID(), bak.desID);
    }
    else
    {
        bak.desID = QString("%1_%2%3").arg(recvValue.module ? PlatformConfigTools::instanceID() : ExtendedConfig::communicationInstanceID(),
                                           bak.desID, recvIDSuffix);
    }

    auto res = INetMsgImpl::send(m_baseImpl->zmqSendSocket, bak, errMsg, ZMQ_DONTWAIT);
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
    /* 这里不处理目标地址了 */
    bak.srcID = m_baseImpl->sendID;

    /* 这里目标地址要换一下,要把发送改为接收 */
    if (bak.desID.endsWith(sendIDSuffix))
    {
        bak.desID = bak.desID.left(bak.desID.size() - sizeof(sendIDSuffix) + 1);
    }
    bak.desID += recvIDSuffix;

    auto res = INetMsgImpl::send(m_baseImpl->zmqSendSocket, bak, errMsg, ZMQ_DONTWAIT);

    return res.success() ? OptionalNotValue() : OptionalNotValue(ErrorCode::DataSendFailed, errMsg);
}

void INetMsg::subscribeCall(const SubscribeFunction& func, const ProtocolPack& pack)
{
    func(pack);
    //    /* 没找到 子选项 */
    //    auto find = m_baseImpl->subscribe.find(pack.operation);
    //    if (find == m_baseImpl->subscribe.end()) {
    //        return;
    //    }
    //    find.value()(pack);
}
