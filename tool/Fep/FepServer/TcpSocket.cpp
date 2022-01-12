#include "TcpSocket.h"
#include "FepInitRecv.h"
#include "FepRecvData.h"
#include "Protocol.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>

class TcpSocketImpl
{
public:
    QByteArray buffer;
    bool isInit{ false };
    RecvContent content;
    int timerID{ -1 };   /* 定时器id */
    int timerCount{ 0 }; /* 定时器计数 */
    QTcpSocket* tcpLink{ nullptr };
    FepInitRecv* initRecv{ nullptr };
    FepRecvData* recvData{ nullptr };
    RecvState state{ RecvState::Wait };
};

TcpSocket::TcpSocket(const QString& workDir, QObject* parent)
    : QObject(parent)
    , m_impl(new TcpSocketImpl)
{
    Q_ASSERT(!workDir.isEmpty());
    m_impl->content.workDir = workDir;
}
TcpSocket::~TcpSocket()
{
    disconnectLink(false);

    m_impl->tcpLink->setParent(nullptr);
    m_impl->initRecv->setParent(nullptr);
    m_impl->recvData->setParent(nullptr);

    delete m_impl->initRecv;
    delete m_impl->recvData;
    delete m_impl->tcpLink;
    delete m_impl;
}
void TcpSocket::stop() { disconnectLink(false); }
void TcpSocket::start(qintptr handle)
{
    if (m_impl->isInit)
    {
        return;
    }
    m_impl->initRecv = new FepInitRecv(&(m_impl->content), nullptr);
    m_impl->recvData = new FepRecvData(&(m_impl->content), nullptr);
    m_impl->tcpLink = new QTcpSocket(nullptr);
    m_impl->tcpLink->setReadBufferSize(DefLen::maxBufferSize);
    m_impl->tcpLink->setSocketDescriptor(handle);
    m_impl->content.srcIP = m_impl->tcpLink->peerAddress().toString();
    m_impl->content.srcPort = m_impl->tcpLink->peerPort();

    connect(m_impl->tcpLink, &QTcpSocket::readyRead, this, &TcpSocket::readyRead);
    connect(m_impl->tcpLink, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &TcpSocket::errorMsg);

    connect(m_impl->initRecv, &FepInitRecv::sg_stateChange, this, &TcpSocket::stateSwitch);
    connect(m_impl->initRecv, &FepInitRecv::sg_sendData, this, &TcpSocket::sendData);

    connect(m_impl->recvData, &FepRecvData::sg_stateChange, this, &TcpSocket::stateSwitch);
    connect(m_impl->recvData, &FepRecvData::sg_sendData, this, &TcpSocket::sendData);
    m_impl->isInit = true;
    m_impl->timerID = startTimer(1000);
}

void TcpSocket::sendData(const QByteArray& sendData)
{
    auto ret = m_impl->tcpLink->write(sendData);
    m_impl->tcpLink->waitForBytesWritten();
    if (ret == -1)
    {
        stateSwitch(RecvState::Abort, "写入数据错误");
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
void TcpSocket::stateSwitch(RecvState state, const QString& msgError)
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
