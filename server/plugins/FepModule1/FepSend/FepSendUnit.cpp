#include "FepSendUnit.h"
#include "FepFileOperate.h"
#include "FepInitSend.h"
#include "FepSendData.h"
#include "IStateSend.h"
#include "Protocol.h"
#include <QThread>

static bool ipv4Verify(const QString& ip)
{
    if (ip.isEmpty())
    {
        return false;
    }
    auto ipList = ip.split(".", QString::SkipEmptyParts);
    if (ipList.size() != 4)
    {
        return false;
    }
    bool isOK = false;
    for (auto& item : ipList)
    {
        auto convert = item.toUInt(&isOK, 10);
        if (!isOK || convert > 255)
        {
            return false;
        }
    }
    return true;
}

class FepSendUnitImpl
{
public:
    QByteArray buffer;
    SendContent content;
    int timerID{ -1 };   /* 定时器id */
    int timerCount{ 0 }; /* 定时器计数 */
    QString errMsg;      /* 错误信息 */
    bool isRun{ false };
    QTcpSocket* tcpLink{ nullptr };
    //状态管理队列
    FepInitSend* initSend{ nullptr };
    FepSendData* sendData{ nullptr };
};

FepSendUnit::FepSendUnit(QObject* parent)
    : QObject(parent)
    , m_impl(new FepSendUnitImpl)
{
    qRegisterMetaType<SendState>("SendState");
    qRegisterMetaType<SendState>("SendState&");
    m_impl->tcpLink = new QTcpSocket(nullptr);

    m_impl->initSend = new FepInitSend(&(m_impl->content), nullptr);
    m_impl->sendData = new FepSendData(&(m_impl->content), nullptr);

    //    qDebug() << "DefLen::typeIDLen        " << DefLen::typeIDLen;
    //    qDebug() << "DefLen::fileNameLen      " << DefLen::fileNameLen;
    //    qDebug() << "DefLen::fileContentLen   " << DefLen::fileContentLen;
    //    qDebug() << "DefLen::dataUnitIDLen    " << DefLen::dataUnitIDLen;
    //    qDebug() << "DefLen::fileIDLen        " << DefLen::fileIDLen;
    //    qDebug() << "DefLen::dataMaxLen       " << DefLen::dataMaxLen;
    //    qDebug() << "DefLen::packLenSendREQ   " << DefLen::packLenSendREQ;
    //    qDebug() << "DefLen::packLenAnserREQ  " << DefLen::packLenAnserREQ;
    //    qDebug() << "DefLen::packLenFinish    " << DefLen::packLenFinish;
    //    qDebug() << "DefLen::packLenData      " << DefLen::packLenData;
    //    qDebug() << "DefLen::packMaxLenData   " << DefLen::packMaxLenData;
    //    qDebug() << "DefLen::packMinLenData   " << DefLen::packMinLenData;
    //    qDebug() << "DefLen::maxBufferSize    " << DefLen::maxBufferSize;
}
FepSendUnit::~FepSendUnit()
{
    disconnectLink(false);

    m_impl->initSend->setParent(nullptr);
    m_impl->sendData->setParent(nullptr);
    m_impl->tcpLink->setParent(nullptr);

    delete m_impl->initSend;
    delete m_impl->sendData;
    delete m_impl->tcpLink;
    delete m_impl;
}
/* 设置服务器的地址信息,在服务启动后此函数调用无效 */
void FepSendUnit::setServerAddr(const QString& dstIP, quint16 dstPort)
{
    if (m_impl->isRun)
    {
        return;
    }
    m_impl->content.dstIP = dstIP;
    m_impl->content.dstPort = dstPort;
}
/* 设置工作目录信息,在服务启动后此函数调用无效 */
void FepSendUnit::setWorkDir(const QString& openDir)
{
    if (m_impl->isRun)
    {
        return;
    }
    m_impl->content.workDir = openDir;
}
bool FepSendUnit::startSendFile(const QString& filePath, QString& errMsg)
{
    auto& content = m_impl->content;
    if (m_impl->isRun)
    {
        errMsg = "发送已在运行";
        return false;
    }
    if (!ipv4Verify(content.dstIP))
    {
        errMsg = "当前IP地址非法";
        return false;
    }
    if (!FepFileOperate::setFile(m_impl->content, filePath, errMsg))
    {
        return false;
    }
    if (m_impl->content.fileList.isEmpty())
    {
        emit sg_msg(false, "");
        return true;
    }

    m_impl->timerID = startTimer(1000);
    qDebug() << "tcp链接初始化";

    m_impl->tcpLink->connectToHost(content.dstIP, content.dstPort);
    m_impl->tcpLink->setReadBufferSize(DefLen::maxBufferSize);

    connect(m_impl->tcpLink, &QTcpSocket::connected, [=]() {
        qDebug() << "tcp链接建立成功";
        stateSwitch(SendState::InitSend, QString());
    });

    connect(m_impl->tcpLink, &QTcpSocket::readyRead, this, &FepSendUnit::readyRead);
    connect(m_impl->tcpLink, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &FepSendUnit::errorMsg);

    connect(m_impl->initSend, &FepInitSend::sg_stateChange, this, &FepSendUnit::stateSwitch);
    connect(m_impl->initSend, &FepInitSend::sg_sendData, this, &FepSendUnit::sendData);

    connect(m_impl->sendData, &FepSendData::sg_stateChange, this, &FepSendUnit::stateSwitch);
    connect(m_impl->sendData, &FepSendData::sg_sendData, this, &FepSendUnit::sendData);
    connect(m_impl->sendData, &FepSendData::sg_timeReset, [=]() { m_impl->timerCount = 0; });

    m_impl->isRun = true;
    return true;
}
bool FepSendUnit::start(QString& errMsg)
{
    auto& content = m_impl->content;
    if (m_impl->isRun)
    {
        errMsg = "发送已在运行";
        return false;
    }
    if (!ipv4Verify(content.dstIP))
    {
        errMsg = "当前IP地址非法";
        return false;
    }
    if (!FepFileOperate::getSendFileList(m_impl->content, errMsg))
    {
        return false;
    }
    if (m_impl->content.fileList.isEmpty())
    {
        emit sg_msg(false, "");
        return true;
    }

    m_impl->timerID = startTimer(1000);
    qDebug() << "tcp链接初始化";

    m_impl->tcpLink->connectToHost(content.dstIP, content.dstPort);
    m_impl->tcpLink->setReadBufferSize(DefLen::maxBufferSize);

    connect(m_impl->tcpLink, &QTcpSocket::connected, [=]() {
        qDebug() << "tcp链接建立成功";
        stateSwitch(SendState::InitSend, QString());
    });

    connect(m_impl->tcpLink, &QTcpSocket::readyRead, this, &FepSendUnit::readyRead);
    connect(m_impl->tcpLink, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &FepSendUnit::errorMsg);

    connect(m_impl->initSend, &FepInitSend::sg_stateChange, this, &FepSendUnit::stateSwitch);
    connect(m_impl->initSend, &FepInitSend::sg_sendData, this, &FepSendUnit::sendData);

    connect(m_impl->sendData, &FepSendData::sg_stateChange, this, &FepSendUnit::stateSwitch);
    connect(m_impl->sendData, &FepSendData::sg_sendData, this, &FepSendUnit::sendData);
    connect(m_impl->sendData, &FepSendData::sg_timeReset, [=]() { m_impl->timerCount = 0; });

    m_impl->isRun = true;
    return true;
}
void FepSendUnit::errorMsg(QAbstractSocket::SocketError sockerError)
{
    if (sockerError == QAbstractSocket::RemoteHostClosedError)
    {
        disconnectLink(false);
        return;
    }

    qDebug() << m_impl->tcpLink->errorString();
    disconnectLink(true);
    emit sg_error(m_impl->tcpLink->errorString());
}
void FepSendUnit::stop() { disconnectLink(false); }

//读取网络数据
void FepSendUnit::readyRead()
{
    m_impl->timerCount = 0;
    auto tempBuffer = m_impl->tcpLink->readAll();
    m_impl->buffer.append(tempBuffer);
    if (m_impl->buffer.isEmpty())
    {
        /* 这个有问题啊,系统错误 */
        stateSwitch(SendState::Abort, "系统错误");
        return;
    }
    auto packSize = m_impl->buffer.size();
    /* 第一个字节是类型 */
    auto type = PackType(m_impl->buffer.at(0));
    switch (type)
    {
    case PackType::SendREQ: /* 发送请求包 */
    case PackType::Data:    /* 数据包 */
    {
        /* 客户端不会收到这个 */
        break;
    }
    case PackType::AnserREQ: /* 请求应答包 */
    {
        static const auto packLen = DefLen::packLenAnserREQ;
        if (packSize < packLen)
        {
            /* 长度不够再等等 */
            return;
        }
        QByteArray byteArray(m_impl->buffer.data(), packLen);
        /* 删除已经读取的 */
        m_impl->buffer = m_impl->buffer.mid(packLen);
        m_impl->initSend->recvResponse(byteArray);

        return;
    }
    case PackType::Finish: /* 结束确认包 */
    {
        static const auto packLen = DefLen::packLenFinish;
        if (packSize < packLen)
        {
            /* 长度不够再等等 */
            return;
        }

        //        QByteArray byteArray(m_impl->buffer.data(), packLen);
        /* 删除已经读取的 */
        m_impl->buffer = m_impl->buffer.mid(packLen);

        /* 客户端这边是一个文件一个文件的发送的,所以这里结束之后直接下一个文件就行了,连包都可以不判断的 */
        //  FinishPack pack;
        //  QString errMsg;
        //  if (!pack.fromByteArray(byteArray, errMsg))
        //  {
        //      stateSwitch(SendState::Error, errMsg);
        //      return;
        //  }
        stateSwitch(SendState::Next);
        return;
    }
    }

    /* 不该来的来了 */
    stateSwitch(SendState::Abort, "数据解析错误");
}

//状态变换
void FepSendUnit::stateSwitch(SendState state, const QString& msgError)
{
    m_impl->content.state = state;
    m_impl->timerCount = 0;
    switch (state)
    {
    case SendState::InitSend: /* 初始化连接 */
    {
        qDebug() << "初始化发送链接";
        m_impl->initSend->sendRequest();
        break;
    }
    case SendState::SendFile: /* 发送文件 */
    {
        /* 发送 发送文件的请求 */
        qDebug() << "发送文件";
        m_impl->sendData->sendRequest();
        break;
    }
    case SendState::Close: /* 正常关闭连接-正常结束 */
    {
        qDebug() << "链接关闭";
        disconnectLink(false);
        emit sg_msg(!msgError.isEmpty(), msgError);
        break;
    }
    case SendState::Next: /* 单个文件发送完成 */
    {
        qDebug() << "下一个文件";
        FepFileOperate::nextFile(m_impl->content);
        stateSwitch(SendState::InitSend, QString());
        break;
    }
    case SendState::Abort: /* 异常 */
    {
        qDebug() << "链接异常关闭";
        disconnectLink(true);
        emit sg_error(msgError);
        break;
    }
    }
}
//发送数据
void FepSendUnit::sendData(const QByteArray& sendData)
{
    auto ret = m_impl->tcpLink->write(sendData);
    m_impl->tcpLink->waitForBytesWritten(30000);
    if (ret == -1)
    {
        stateSwitch(SendState::Abort, "写入数据错误");
    }
}
void FepSendUnit::disconnectLink(bool abort)
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
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
    m_impl->initSend->disconnect();
    m_impl->sendData->disconnect();
    m_impl->buffer = QByteArray();
    m_impl->content = SendContent();
    m_impl->isRun = false;
    m_impl->errMsg = QString();
    m_impl->timerCount = 0;
    qDebug() << "链接断开";
}
void FepSendUnit::timerEvent(QTimerEvent* /*event*/)
{
    ++m_impl->timerCount;
    if (m_impl->timerCount > 5)
    {
        qDebug() << "超时断开连接";
        disconnectLink(true);
    }
}
