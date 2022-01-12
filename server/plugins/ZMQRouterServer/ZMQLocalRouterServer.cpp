#include "ZMQLocalRouterServer.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "PackDefine.h"
#include "PlatformInterface.h"
#include "SocketPackHelper.h"
#include "Utility.h"
#include "ZMQUtility.h"
#include <QDebug>
#include <QUuid>
#include <QtConcurrent>
#include <atomic>

using RouterConvert = void (*)(ProtocolPack&);
class ZMQLocalRouterServerImpl
{
public:
    /* 运行标志 true代表运行 */
    std::atomic_bool runningFlag{ false };
    /* 此三个数据用于实现本地和远端的数据交互 */
    QString convertRecvID;
    QString convertSendID;
    QString convertAddr;
    QString masterIP;

    cppmicroservices::BundleContext context;

    /* zmq的上下文 */
    // ZMQContext zmqContext;

    inline ZMQLocalRouterServerImpl()
    {
        convertRecvID = QString("%1%2").arg(PlatformConfigTools::instanceID(), SocketPackHelper::getRecvIDSuffix());
        convertSendID = QString("%1%2").arg(PlatformConfigTools::instanceID(), SocketPackHelper::getSendIDSuffix());
    }
    inline ~ZMQLocalRouterServerImpl() {}

public:
    /* 内部到内部或者内部到外部 */
    static void routerTo(ZMQSocket& recv, ZMQSocket& to);
    /* 接收外部的信息 */
    static void dealerToRouterRecv(ZMQSocket& recv, ZMQSocket& to);
};
void ZMQLocalRouterServerImpl::routerTo(ZMQSocket& from, ZMQSocket& to)
{
    ProtocolPack pack;
    QString errMsg;
    auto res = SocketPackHelper::routerRecv(from, pack, errMsg);
    /* 没收到任何消息 */
    if (res.eagain())
    {
        return;
    }
    /* 产生了错误 */
    else if (!res.success())
    {
        qWarning() << QString("ZMQLocalRouterServer Recv Data Error：%1").arg(errMsg);
        return;
    }

    if (pack.module)
    {
        /* 本地发送 */
        res = SocketPackHelper::routerSend(from, pack, errMsg);
    }
    else
    {
#ifndef RUN_ENV_SERVER
#define RUN_ENV_SERVER 0
#endif

#if (RUN_ENV_SERVER == 1)
        if (pack.cascadeACKAddr.isEmpty())
        {
            qWarning() << QString("ZMQLocalRouterServer Protocol error");
            return;
        }
        /* 转换地址 */
        pack.cascadeDesID = QString("%1%2").arg(pack.cascadeACKAddr, SocketPackHelper::getRecvIDSuffix());

#else
        /* 客户端发送的目的地址 */
        static auto cascadeDesID = QString("%1%2").arg(ExtendedConfig::communicationInstanceID(), SocketPackHelper::getRecvIDSuffix());
        pack.cascadeDesID = cascadeDesID;
#endif
        /* 对外发送需要使用级联地址 */
        res = SocketPackHelper::dealerSendTotalData(to, pack, errMsg);
    }

    /* 没收到任何消息 */
    if (res.eagain())
    {
        return;
    }
    /* 产生了错误 */
    else if (!res.success())
    {
        qWarning() << QString("ZMQLocalRouterServer Send Data Error：%1，srcID：%2，desID：%3").arg(errMsg, pack.srcID, pack.desID);
        return;
    }
}
void ZMQLocalRouterServerImpl::dealerToRouterRecv(ZMQSocket& recv, ZMQSocket& to)
{
    QString errMsg;
    ProtocolPack pack;
    auto res = SocketPackHelper::dealerRecvTotalData(recv, pack, errMsg);
    /* 没收到任何消息 */
    if (res.eagain())
    {
        return;
    }
    /* 产生了错误 */
    else if (!res.success())
    {
        qWarning() << QString("ZMQLocalRouterServer Recv Data Error：%1").arg(errMsg);
        return;
    }

    /* 这个里面收到的不能是模块间通讯 */
    if (pack.module)
    {
        qWarning() << QString("ZMQLocalRouterServer cannot be inter module communication");
        return;
    }
    /* 处理级联协议地址 */
    pack.cascadeACKAddr = pack.cascadeSrcID;
    pack.cascadeACKAddr = SocketPackHelper::removeSendIDSuffix(pack.cascadeACKAddr);

    if (pack.cascadeACKAddr.endsWith(SocketPackHelper::getSendIDSuffix())) {}
    res = SocketPackHelper::routerSend(to, pack, errMsg);
    /* 没收到任何消息 */
    if (res.eagain())
    {
        return;
    }
    /* 产生了错误 */
    else if (!res.success())
    {
        qWarning() << QString("ZMQRouterServer Send Data Error：%1，srcID：%2，desID：%3").arg(errMsg, pack.srcID, pack.desID);
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
ZMQLocalRouterServer::ZMQLocalRouterServer(cppmicroservices::BundleContext context)
    : QThread(nullptr)
    , m_impl(new ZMQLocalRouterServerImpl)
{
    m_impl->context = context;
}

ZMQLocalRouterServer::~ZMQLocalRouterServer()
{
    stopServer();
    delete m_impl;
    m_impl = nullptr;
}

void ZMQLocalRouterServer::stopServer()
{
    if (!isRunning())
    {
        return;
    }
    m_impl->runningFlag = false;
    quit();
    wait();
    // m_impl->zmqContext.destroy();
}

bool ZMQLocalRouterServer::startServer(const QString& convertAddr, QString* errMsg)
{
    /* 已经在运行 */
    if (isRunning())
    {
        Utility::nullAssignment(errMsg, "服务已经运行");
        return false;
    }
    auto zmqContextResult = SocketPackHelper::registerZMQContext();
    if (!zmqContextResult)
    {
        Utility::nullAssignment(errMsg, zmqContextResult.msg());
        return false;
    }

    m_impl->convertAddr = convertAddr;

    start();
    return true;
}
void ZMQLocalRouterServer::run()
{
    /* 退出后flag自动归位 */
    AtomicFlagHelper runFlag(m_impl->runningFlag, true);

    ZMQSocket localSocket;        /* 本地的路由 */
    ZMQSocket externalRecvSocket; /* 外部 */
    ZMQSocket externalSendSocket; /* 外部 */

    auto zmqContextResult = SocketPackHelper::getZMQContext();
    if (!zmqContextResult)
    {
        qWarning() << zmqContextResult.msg();
        return;
    }
    auto zmqContext = zmqContextResult.value();

    auto socketInit = SocketPackHelper::initDealerSocket(*zmqContext, externalRecvSocket, m_impl->convertAddr, m_impl->convertRecvID);
    if (!socketInit)
    {
        qWarning() << "ZMQLocalRouterServer ExternalRecvSocket init error:" << socketInit.msg();
        return;
    }
    socketInit = SocketPackHelper::initDealerSocket(*zmqContext, externalSendSocket, m_impl->convertAddr, m_impl->convertSendID);
    if (!socketInit)
    {
        qWarning() << "ZMQLocalRouterServer ExternalSendSocket init error:" << socketInit.msg();
        return;
    }
    socketInit = SocketPackHelper::initRouterSocket(*zmqContext, localSocket, ZMQHelper::deaultLocalAddress(), "localSocket");
    if (!socketInit)
    {
        qWarning() << "ZMQLocalRouterServer LocalSocket init error:" << socketInit.msg();
        return;
    }

    /* 初始化轮询集合 */
    zmq_pollitem_t items[2] = { { localSocket.handle(), 0, ZMQ_POLLIN, 0 }, { externalRecvSocket.handle(), 0, ZMQ_POLLIN, 0 } };

    while (m_impl->runningFlag)
    {
        auto rc = zmq_poll(&(items[0]), 2, SocketPackHelper::getTimeo());
        // 没有消息
        if (rc == 0)
        {
            continue;
        }
        /* 发生错误 */
        else if (rc <= 0)
        {
            qWarning() << "ZMQLocalRouterServer poll error:" << ZMQHelper::errMsg();
            return;
        }

        if (items[0].revents & ZMQ_POLLIN)
        {
            ZMQLocalRouterServerImpl::routerTo(localSocket, externalSendSocket);
        }
        if (items[1].revents & ZMQ_POLLIN)
        {
            ZMQLocalRouterServerImpl::dealerToRouterRecv(externalRecvSocket, localSocket);
        }
    }
}
