#include "FepServer.h"
#include "FepTcpServer.h"
#include "FepTcpSocket.h"
#include "MessagePublish.h"
#include <QDir>
#include <QThread>

class FepServerImpl
{
public:
    QMap<QString, QVariantMap> ipcheckData;
    QString workDir{ "FepRecv" };  // Fep的工作目录
    QString desDir{ "DesRecv" };   // 接收完成后的目标目录
    FepTcpServer* tcpServer{ nullptr };
    quint16 port{ 5555 };
    bool isRun{ false };

public:
    static bool createWorkDir(QString& workDir, QString& errMsg);
};
//读取文件夹里面的文件
bool FepServerImpl::createWorkDir(QString& workDir, QString& errMsg)
{
    if (workDir.isEmpty())
    {
        errMsg = "目录路径为空";
        return false;
    }
    QDir dir;
    if (!dir.exists(workDir))
    {
        if (!dir.mkpath(workDir))
        {
            errMsg = "目录创建失败";
            return false;
        }
    }
    dir.setCurrent(workDir);
    workDir = dir.absolutePath();
    return true;
}
FepServer::FepServer(QObject* parent)
    : QObject(parent)
    , m_impl(new FepServerImpl)
{
    qRegisterMetaType<qintptr>("qintptr");
    m_impl->tcpServer = new FepTcpServer(nullptr);
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
void FepServer::setWorkDir(const QString& fepDir, const QString& desDir)
{
    Q_ASSERT(!fepDir.isEmpty());
    Q_ASSERT(!desDir.isEmpty());
    m_impl->workDir = fepDir;
    m_impl->desDir = desDir;
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
    if (!FepServerImpl::createWorkDir(m_impl->workDir, errMsg))
    {
        return false;
    }

    if (!FepServerImpl::createWorkDir(m_impl->desDir, errMsg))
    {
        return false;
    }

    SystemLogPublish::infoMsg(QString("FEP当前工作目录:%1").arg(m_impl->workDir));
    SystemLogPublish::infoMsg(QString("FEP当前目标目录:%1").arg(m_impl->desDir));
    SystemLogPublish::infoMsg(QString("FEP当前监视端口:%1").arg(m_impl->port));

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
    connect(m_impl->tcpServer, &FepTcpServer::sg_newConnect, this, &FepServer::doNewClientNew);

    m_impl->isRun = true;
    return true;
}

void FepServer::stop()
{
    m_impl->isRun = false;
    m_impl->tcpServer->disconnect();
    m_impl->tcpServer->close();
}

// get new
void FepServer::doNewClientNew(qintptr handle)
{
    auto socket = new FepTcpSocket(m_impl->workDir, m_impl->desDir);

    connect(socket, &FepTcpSocket::finished, socket, &FepTcpSocket::deleteLater);
    if (!socket->startFep(handle, m_impl->ipcheckData))
    {
        delete socket;
        return;
    }
}
