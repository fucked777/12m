#include "ZMQRouterServer.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "HeartbeatMessageSerialize.h"
#include "SubscriberHelper.h"
#include "MessagePublish.h"
#include "PackDefine.h"
#include "SocketPackHelper.h"
#include "Utility.h"
#include "ZMQUtility.h"
#include <QApplication>
#include <QDebug>
#include <QUuid>
#include <QtConcurrent>
#include <atomic>

static constexpr int timeo = 1000;
using RouterConvert = void (*)(ProtocolPack&);
class ZMQRouterServerImpl
{
    DIS_COPY_MOVE(ZMQRouterServerImpl)
public:
    /* 运行标志 true代表运行 */
    std::atomic_bool runningFlag{ false };
    QString frontendAddr;
    QString backendAddr;
    cppmicroservices::BundleContext context;

    MasterSlaveStatus masterStatus{ MasterSlaveStatus::Unknown };

    inline ZMQRouterServerImpl() {}
    inline ~ZMQRouterServerImpl() {}

public:
    static void routerRecv(ZMQSocket& frontend, ZMQSocket& backend);
};

void ZMQRouterServerImpl::routerRecv(ZMQSocket& from, ZMQSocket& to)
{
    ProtocolPack pack;
    QString errMsg;
    auto res = SocketPackHelper::routerRecvTotalData(from, pack, errMsg);
    /* 没收到任何消息 */
    if (res.eagain())
    {
        return;
    }
    /* 产生了错误 */
    else if (!res.success())
    {
        qWarning() << QString("ZMQRouterServer Recv Data Error：%1").arg(errMsg);
        return;
    }

    res = SocketPackHelper::routerSendTotalData(to, pack, errMsg);
    /* 没收到任何消息 */
    if (res.eagain())
    {
        return;
    }
    /* 产生了错误 */
    else if (!res.success())
    {
        qWarning() << QString("ZMQRouterServer Send Data Error：%1，srcID：%2，desID：%3").arg(errMsg, pack.cascadeSrcID, pack.cascadeDesID);
        return;
    }
}
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
ZMQRouterServer::ZMQRouterServer(cppmicroservices::BundleContext context)
    : QThread(nullptr)
    , m_impl(new ZMQRouterServerImpl)
{
    m_impl->context = context;
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::sg_masterSlave, this, &ZMQRouterServer::masterSlave);
}

ZMQRouterServer::~ZMQRouterServer()
{
    disconnect(SubscriberHelper::getInstance(), &SubscriberHelper::sg_masterSlave, this, &ZMQRouterServer::masterSlave);
    stopServer();
    delete m_impl;
    m_impl = nullptr;
}
void ZMQRouterServer::stopServer()
{
    if (!isRunning())
    {
        return;
    }
    m_impl->runningFlag = false;
    quit();
    while (!isFinished())
    {
        QApplication::processEvents();
        QThread::msleep(10);
    }
    wait();
}

bool ZMQRouterServer::startServer(const QString& frontend, const QString& backend, QString* errMsg)
{
    /* 已经在运行 */
    if (isRunning())
    {
        Utility::nullAssignment(errMsg, "服务已经运行");
        return false;
    }

    m_impl->frontendAddr = frontend;
    m_impl->backendAddr = backend;

    return true;
}

void ZMQRouterServer::run()
{
    /* 退出后flag自动归位 */
    AtomicFlagHelper runFlag(m_impl->runningFlag, true);

    /* zmq的上下文 */
    ZMQContext zmqContext;

    /* 接收的数据包 */
    ZMQSocket frontend; /* 前端socket接收或者发送对外的包 */
    ZMQSocket backend;  /* 后端socket 接收或者发送本地的包 */

    /* 未初始化的情况才去初始化context */
    if (!zmqContext.init())
    {
        qWarning() << "ZMQRouterServer ZMAContext Init Error:" << zmqContext.getErrMsg();
        return;
    }

    if (!frontend.init(zmqContext, ZMQ_ROUTER))
    {
        qWarning() << "ZMQRouterServer Frontend Error:" << frontend.getErrMsg();
        return;
    }
    if (!backend.init(zmqContext, ZMQ_ROUTER))
    {
        qWarning() << "ZMQRouterServer Backend Error:" << backend.getErrMsg();
        return;
    }

    /* recv超时时间,单位毫秒 -1会无限等待 */
    // frontend.set(ZMQ_RCVTIMEO, timeo);
    // backend.set(ZMQ_RCVTIMEO, timeo);
    /* send超时时间,单位毫秒 -1会无限等待 */
    // frontend.set(ZMQ_SNDTIMEO, timeo);
    // backend.set(ZMQ_SNDTIMEO, timeo);
    /*
     * 默认属性值是0，表示会静默的丢弃不能路由到的消息。属性值是1时，
     * 如果消息不能路由，会返回一个EHOSTUNREACH错误代码
     */
    frontend.set(ZMQ_ROUTER_MANDATORY, 1);
    backend.set(ZMQ_ROUTER_MANDATORY, 1);

    /* 开启TCP保活机制，防止网络连接因长时间无数据而被中断 */
    frontend.set(ZMQ_TCP_KEEPALIVE, 1);
    backend.set(ZMQ_TCP_KEEPALIVE, 1);

    /* 网络连接空闲1min即发送保活包 */
    frontend.set(ZMQ_TCP_KEEPALIVE_IDLE, 3);
    backend.set(ZMQ_TCP_KEEPALIVE_IDLE, 3);

    /* 设置ID */
    frontend.set(ZMQ_IDENTITY, "frontend");
    backend.set(ZMQ_IDENTITY, "backend");

    /* 绑定 */
    if (!frontend.bind(m_impl->frontendAddr))
    {
        qWarning() << QString("ZMQRouterServer Backend Error:%1 addr %2").arg(frontend.getErrMsg(), m_impl->frontendAddr);
        return;
    }

    if (!backend.bind(m_impl->backendAddr))
    {
        qWarning() << QString("ZMQRouterServer Backend Error:%1 addr %2").arg(backend.getErrMsg(), m_impl->backendAddr);
        return;
    }

    /* 初始化轮询集合 */
    zmq_pollitem_t items[] = { { frontend.handle(), 0, ZMQ_POLLIN, 0 }, { backend.handle(), 0, ZMQ_POLLIN, 0 } };

    while (m_impl->runningFlag)
    {
        auto rc = zmq_poll(&(items[0]), 2, timeo);
        // 没有消息
        if (rc == 0)
        {
            continue;
        }
        /* 发生错误 */
        else if (rc <= 0)
        {
            qWarning() << "ZMQRouterServer Poll Error:" << ZMQHelper::errMsg();
            return;
        }

        // frontend有数据来
        if (items[0].revents & ZMQ_POLLIN)
        {
            ZMQRouterServerImpl::routerRecv(frontend, backend);
        }
        // backend有数据来
        if (items[1].revents & ZMQ_POLLIN)
        {
            ZMQRouterServerImpl::routerRecv(backend, frontend);
        }
    }
}

void ZMQRouterServer::masterSlave(const HeartbeatMessage& msg)
{
    if (m_impl->masterStatus == msg.status)
    {
        return;
    }
    m_impl->masterStatus = msg.status;
    /* 主机先停止再启动
     * 备机直接停止
     */

    if (msg.status == MasterSlaveStatus::Master)
    {
        SystemLogPublish::infoMsg(QString("主备切换:%1主机路由重置中").arg(msg.currentMasterIP));
        stopServer();
        start();
        SystemLogPublish::infoMsg(QString("主备切换:%1主机路由重置完成").arg(msg.currentMasterIP));
    }
    else
    {
        SystemLogPublish::infoMsg(QString("主备切换:备机路由停止中"));
        stopServer();
        SystemLogPublish::infoMsg(QString("主备切换:备机路由停止完成"));
    }
}
