#include "TcpSocket.h"
#include "FepInitRecv.h"
#include "FepProtocolSerialize.h"
#include "FepRecvData.h"
#include "MessagePublish.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>

class TcpSocketImpl
{
public:
    QByteArray buffer;      /* 内部的缓存 */
    FepRecvContent content; /* 上下文 */
    QTcpSocket tcpLink;     /* 链接的socket */
    QVariantMap userData;   /* 外部传入的用户数据 */

    FepInitRecv* initRecv{ nullptr };
    FepRecvData* recvData{ nullptr };
    FepRecvState state{ FepRecvState::Wait };
};

TcpSocket::TcpSocket(const QString& workDir, QObject* parent)
    : QThread(parent)
    , m_impl(new TcpSocketImpl)
{
    Q_ASSERT(!workDir.isEmpty());
    m_impl->content.workDir = workDir;
}
TcpSocket::~TcpSocket()
{
    //    disconnectLink(false);

    //    m_impl->tcpLink->setParent(nullptr);
    //    m_impl->initRecv->setParent(nullptr);
    //    m_impl->recvData->setParent(nullptr);

    //    delete m_impl->initRecv;
    //    delete m_impl->recvData;
    //    delete m_impl->tcpLink;
    delete m_impl;
}
void TcpSocket::stopFep() { disconnectLink(false); }
bool TcpSocket::startFep(qintptr handle, const QMap<QString, QVariantMap>& ipCheckData)
{
    if (isRunning())
    {
        return false;
    }

    m_impl->tcpLink.setReadBufferSize(ProtocolLen::fepPackMaxData);
    m_impl->tcpLink.setSocketDescriptor(handle);
    m_impl->content.srcIP = m_impl->tcpLink.peerAddress().toString();
    m_impl->content.srcPort = m_impl->tcpLink.peerPort();

    /* 20210818 wp??
     * 这里做一个检查
     * 断开非法IP
     */
    if (!ipCheckData.isEmpty())
    {
        /* 非法IP,直接干掉就行 */
        if (!ipCheckData.contains(m_impl->content.srcIP))
        {
            return false;
        }
        m_impl->userData = ipCheckData.value(m_impl->content.srcIP);
    }
    else
    {
        m_impl->userData["ip"] = (m_impl->content.srcIP);
        m_impl->userData["port"] = (m_impl->content.srcPort);
    }

    SystemLogPublish::infoMsg(QString("Fep模块,中心发送连接成功, ip:%1 port:%2").arg(m_impl->content.srcIP).arg(m_impl->content.srcPort));

    QString ip, port;

    if (m_impl->userData.contains("ip"))
    {
        ip = m_impl->userData["ip"].toString();
    }
    if (m_impl->userData.contains("port"))
    {
        port = m_impl->userData["port"].toString();
    }
    SystemLogPublish::infoMsg(QString("Fep模块,应答 ip:%1 port:%2").arg(ip).arg(port));

    start();

    // m_impl->initRecv = new FepInitRecv(&(m_impl->content), nullptr);
    // m_impl->recvData = new FepRecvData(&(m_impl->content), nullptr);

    // connect(m_impl->initRecv, &FepInitRecv::sg_stateChange, this, &TcpSocket::stateSwitch);
    // connect(m_impl->initRecv, &FepInitRecv::sg_sendData, this, &TcpSocket::sendData);

    // connect(m_impl->recvData, &FepRecvData::sg_stateChange, this, &TcpSocket::stateSwitch);
    // connect(m_impl->recvData, &FepRecvData::sg_sendData, this, &TcpSocket::sendData);
    // m_impl->timerID = startTimer(1000);
}
void TcpSocket::run()
{
    auto tempBuffer = m_impl->tcpLink.readAll();
    m_impl->buffer.append(tempBuffer);
    if (m_impl->buffer.isEmpty())
    {
        /* 这个有问题啊,系统错误 */
        stateSwitch(FepRecvState::Abort, "系统错误");
        return;
    }
    for (;;)
    {
        auto packSize = m_impl->buffer.size();
        if (packSize <= 0)
        {
            break;
        }
        FepSendREQ
            /* 第一个字节是类型 */
            auto type = PackType(m_impl->buffer.at(0));
        switch (type)
        {
        case PackType::SendREQ: /* 发送请求包 */
        {
            static const auto packLen = DefLen::packLenSendREQ;
            if (packSize < packLen)
            {
                /* 长度不够再等等 */
                return;
            }
            QByteArray byteArray(m_impl->buffer.data(), packLen);
            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(packLen);
            m_impl->initRecv->recvResponse(byteArray);
            return;
        }
        case PackType::Data: /* 数据包 */
        {
            /* 数据长度是动态的,这个长度是最小的长度 */
            if (packSize < DefLen::packMinLenData)
            {
                /* 长度不够再等等 */
                return;
            }
            DataPack tempPack;
            QString errMsg;
            quint32 readLen = 0;
            auto res = tempPack.fromByteArray(m_impl->buffer, m_impl->content, readLen, errMsg);
            if (res < 0)
            {
                stateSwitch(RecvState::Abort, errMsg);
                return;
            }
            else if (res == 0)
            {
                /* 长度不够再等等 */
                return;
            }

            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(readLen);
            m_impl->recvData->recvResponse(tempPack);
            return;
        }
        case PackType::AnserREQ: /* 请求应答包 */
        case PackType::Finish:   /* 结束确认包 */
        {
            break;
        }
        }

        /* 不该来的来了 */
        stateSwitch(RecvState::Abort, "数据解析错误");
    }
}
void TcpSocket::sendData(const QByteArray& sendData)
{
    auto ret = m_impl->tcpLink.write(sendData);
    m_impl->tcpLink.waitForBytesWritten();
    if (ret == -1)
    {
        stateSwitch(RecvState::Abort, "写入数据错误");
    }
    for (;;)
    {
        auto packSize = m_impl->buffer.size();
        if (packSize <= 0)
        {
            break;
        }
        /* 第一个字节是类型 */
        auto type = PackType(m_impl->buffer.at(0));
        switch (type)
        {
        case PackType::SendREQ: /* 发送请求包 */
        {
            static const auto packLen = DefLen::packLenSendREQ;
            if (packSize < packLen)
            {
                /* 长度不够再等等 */
                return;
            }
            QByteArray byteArray(m_impl->buffer.data(), packLen);
            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(packLen);
            m_impl->initRecv->recvResponse(byteArray);
            return;
        }
        case PackType::Data: /* 数据包 */
        {
            /* 数据长度是动态的,这个长度是最小的长度 */
            if (packSize < DefLen::packMinLenData)
            {
                /* 长度不够再等等 */
                return;
            }
            DataPack tempPack;
            QString errMsg;
            quint32 readLen = 0;
            auto res = tempPack.fromByteArray(m_impl->buffer, m_impl->content, readLen, errMsg);
            if (res < 0)
            {
                stateSwitch(RecvState::Abort, errMsg);
                return;
            }
            else if (res == 0)
            {
                /* 长度不够再等等 */
                return;
            }

            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(readLen);
            m_impl->recvData->recvResponse(tempPack);
            return;
        }
        case PackType::AnserREQ: /* 请求应答包 */
        case PackType::Finish:   /* 结束确认包 */
        {
            break;
        }
        }

        /* 不该来的来了 */
        stateSwitch(RecvState::Abort, "数据解析错误");
    }
}

void TcpSocket::readyRead()
{
    m_impl->timerCount = 0;
    auto tempBuffer = m_impl->tcpLink->readAll();
    m_impl->buffer.append(tempBuffer);
    if (m_impl->buffer.isEmpty())
    {
        /* 这个有问题啊,系统错误 */
        stateSwitch(RecvState::Abort, "系统错误");
        return;
    }
    for (;;)
    {
        auto packSize = m_impl->buffer.size();
        if (packSize <= 0)
        {
            break;
        }
        /* 第一个字节是类型 */
        auto type = PackType(m_impl->buffer.at(0));
        switch (type)
        {
        case PackType::SendREQ: /* 发送请求包 */
        {
            static const auto packLen = DefLen::packLenSendREQ;
            if (packSize < packLen)
            {
                /* 长度不够再等等 */
                return;
            }
            QByteArray byteArray(m_impl->buffer.data(), packLen);
            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(packLen);
            m_impl->initRecv->recvResponse(byteArray);
            return;
        }
        case PackType::Data: /* 数据包 */
        {
            /* 数据长度是动态的,这个长度是最小的长度 */
            if (packSize < DefLen::packMinLenData)
            {
                /* 长度不够再等等 */
                return;
            }
            DataPack tempPack;
            QString errMsg;
            quint32 readLen = 0;
            auto res = tempPack.fromByteArray(m_impl->buffer, m_impl->content, readLen, errMsg);
            if (res < 0)
            {
                stateSwitch(RecvState::Abort, errMsg);
                return;
            }
            else if (res == 0)
            {
                /* 长度不够再等等 */
                return;
            }

            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(readLen);
            m_impl->recvData->recvResponse(tempPack);
            return;
        }
        case PackType::AnserREQ: /* 请求应答包 */
        case PackType::Finish:   /* 结束确认包 */
        {
            break;
        }
        }

        /* 不该来的来了 */
        stateSwitch(RecvState::Abort, "数据解析错误");
    }
}
void TcpSocket::errorMsg(QAbstractSocket::SocketError sockerError)
{
    if (sockerError == QAbstractSocket::RemoteHostClosedError)
    {
        disconnectLink(false);
        emit sg_destroyed();
        return;
    }

    qDebug() << "errorMsg" << m_impl->tcpLink->errorString();
    emit sg_error(m_impl->tcpLink->errorString());
    disconnectLink(true);
    emit sg_destroyed();
}
void TcpSocket::disconnectLink(bool abort)
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
    if (!m_impl->isInit)
    {
        return;
    }
    m_impl->isInit = false;
    if (abort)
    {
        m_impl->tcpLink->abort();
    }
    else
    {
        m_impl->tcpLink->disconnectFromHost();
        if (m_impl->tcpLink->state() != QAbstractSocket::UnconnectedState)
        {
            if (!m_impl->tcpLink->waitForDisconnected(30000))
            {
                m_impl->tcpLink->abort();
            }
        }
    }
    m_impl->tcpLink->disconnect();
    m_impl->initRecv->disconnect();
    m_impl->recvData->disconnect();
    m_impl->buffer = QByteArray();
    m_impl->content.reset();
    m_impl->state = RecvState::Wait;
    m_impl->timerCount = 0;
}
void TcpSocket::stateSwitch(FepRecvState state, const QString& msgError)
{
    m_impl->timerCount = 0;
    switch (state)
    {
    case RecvState::FullFile: /* 完整的文件 */
    {
        /* 发送 发送文件的请求 */
        qDebug() << "文件已存在,等待下一个文件";
        auto bak = m_impl->content.workDir;
        m_impl->content.reset();
        m_impl->content.workDir = bak;
        break;
    }
    case RecvState::Wait: /* 等待文件接收 */
    {
        /* 发送 发送文件的请求 */
        qDebug() << "等待文件接收";
        m_impl->recvData->sendRequest();
        auto bak = m_impl->content.workDir;
        m_impl->content.reset();
        m_impl->content.workDir = bak;
        // TODO
        m_impl->userData["path"] = m_impl->initRecv->fileName();
        emit sg_oneFileRecvSuccess(m_impl->userData);
        break;
    }
    case RecvState::Abort: /* 异常结束 */
    {
        qDebug() << "异常结束";
        disconnectLink(true);
        emit sg_error(msgError);
        emit sg_destroyed();
        break;
    }
    case RecvState::Fault: /* 接收故障 */
    {
        qDebug() << "正常结束";
        disconnectLink(false);
        emit sg_error(msgError);
        emit sg_destroyed();
        break;
    }
    }
}
/* 定时器超时 */
void TcpSocket::timerEvent(QTimerEvent* /*event*/)
{
    ++m_impl->timerCount;
    if (m_impl->timerCount > 5)
    {
        qDebug() << "超时未响应,连接断开";
        disconnectLink(true);
        emit sg_destroyed();
    }
}
