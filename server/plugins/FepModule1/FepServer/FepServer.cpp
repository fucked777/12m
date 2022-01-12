#include "FepServer.h"
#include "FepFileOperate.h"
#include "TcpServer.h"
#include "TcpSocket.h"
#include <QDir>
#include <QThread>

class FepServerImpl
{
public:
    QMap<QString, QVariantMap> ipcheckData;
    QString workDir{ "FepRecv" };
    QSet<TcpSocket*> newClientSet;      /* 连入的所有客户端  */
    QList<TcpSocket*> removeClientList; /* 需要删除的客户端 */
    TcpServer* tcpServer{ nullptr };
    quint16 port{ 5555 };
    int timerID{ -1 };
    bool isRun{ false };
};

FepServer::FepServer(QObject* parent)
    : QObject(parent)
    , m_impl(new FepServerImpl)
{
    qRegisterMetaType<qintptr>("qintptr");
    m_impl->tcpServer = new TcpServer(nullptr);
}
FepServer::~FepServer()
{
    stop();

    m_impl->tcpServer->setParent(nullptr);
    delete m_impl->tcpServer;
    delete m_impl;
}

/* 设置服务器的地址信息,在服务启动后此函数调用无效 */
void FepServer::setPort(quint16 dstPort) { m_impl->port = dstPort; }
/* 设置工作目录信息,在服务启动后此函数调用无效 */
void FepServer::setWorkDir(const QString& openDir)
{
    Q_ASSERT(!openDir.isEmpty());
    m_impl->workDir = openDir;
}
void FepServer::setIPCheck(const QMap<QString, QVariantMap>& checkData) { m_impl->ipcheckData = checkData; }

/*  */
bool FepServer::start(QString& errMsg)
{
    if (m_impl->isRun)
    {
        errMsg = "服务已在运行";
        return false;
    }
    if (!FepFileOperate::createWorkDir(m_impl->workDir, errMsg))
    {
        return false;
    }

    qDebug() << "FEP当前工作目录:" << m_impl->workDir;
    qDebug() << "FEP当前监视端口:" << m_impl->port;

    /*
     * 20210824
     * 这里Any会同时监听IPV6的地址
     * 所以再后续获取IP地址的时候会得到
     * ipv6兼容ipv4的地址
     * 类似这样--->  ::ffff:192.20.1.156
     */
    if (!m_impl->tcpServer->listen(QHostAddress::AnyIPv4, m_impl->port))
    {
        errMsg = m_impl->tcpServer->errorString();
        return false;
    }
    connect(m_impl->tcpServer, &TcpServer::sg_newConnect, this, &FepServer::doNewClientNew);

    m_impl->isRun = true;
    m_impl->timerID = startTimer(2000);
    return true;
}
template<typename T>
static void stopAll(T& value)
{
    for (auto& item : value)
    {
        delete item;
    }
    value.clear();
}
void FepServer::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
    stopAll(m_impl->newClientSet);
    stopAll(m_impl->removeClientList);
    m_impl->tcpServer->disconnect();
    m_impl->tcpServer->close();
}

// get new
void FepServer::doNewClientNew(qintptr handle)
{
    auto thread = new QThread(nullptr);
    auto socket = new TcpSocket(m_impl->workDir, nullptr);
    socket->moveToThread(thread);
    m_impl->newClientSet.insert(socket);

    connect(socket, &TcpSocket::sg_destroyed, [=]() {
        auto find = m_impl->newClientSet.find(socket);
        if (find == m_impl->newClientSet.end())
        {
            return;
        }
        m_impl->newClientSet.erase(find);
        socket->stop();
        m_impl->removeClientList.append(socket);
        //        thread->disconnect();
        //        thread->quit();
        //        thread->wait();
        //        thread->deleteLater();
    });
    connect(socket, &TcpSocket::sg_oneFileRecvSuccess, this, &FepServer::sg_oneFileRecvSuccess);
    connect(this, &FepServer::sg_newConnectStart, socket, &TcpSocket::start);

    thread->start();
    emit sg_newConnectStart(handle, m_impl->ipcheckData);
}

/* 定时器超时 */
void FepServer::timerEvent(QTimerEvent* /*event*/) { stopAll(m_impl->removeClientList); }
