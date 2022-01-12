#include "FepTcpSocket.h"
#include "FepProtocolSerialize.h"
#include "MessagePublish.h"
#include <QApplication>
#include <QDir>
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>

class FepTcpSocketImpl
{
public:
    QByteArray buffer;      /* 内部的缓存 */
    FepRecvContent content; /* 上下文 */
    QVariantMap userData;   /* 外部传入的用户数据 */

    qintptr handle{};                       /* 传入的socket指针 */
    QMap<QString, QVariantMap> ipCheckData; /* 检查IP的 */

    FepRecvState state{ FepRecvState::Wait };

public:
    static bool writeToFile(FepRecvContent&, const QByteArray&, QString& errMsg);
    static quint16 createFileID(const QString& /*fileName*/);
};

bool FepTcpSocketImpl::writeToFile(FepRecvContent& content, const QByteArray& writeData, QString& errMsg)
{
    if (!content.file.isOpen())
    {
        content.file.setFileName(content.recvFileInfo.absoluteFilePath);
        if (!content.file.open(QFile::WriteOnly | QFile::Append))
        {
            errMsg = QString("Fep:文件打开失败 %1").arg(content.file.errorString());
            return false;
        }
    }

    auto size = content.file.write(writeData);
    if (size < 0 || size != writeData.size())
    {
        errMsg = QString("Fep:数据写入失败 %1").arg(content.file.errorString());
        return false;
    }

    return true;
}
quint16 FepTcpSocketImpl::createFileID(const QString& /*fileName*/) { return static_cast<quint16>(std::rand()); }

FepTcpSocket::FepTcpSocket(const QString& workDir, const QString& desDir, QObject* parent)
    : QThread(parent)
    , m_impl(new FepTcpSocketImpl)
{
    Q_ASSERT(!workDir.isEmpty());
    Q_ASSERT(!desDir.isEmpty());
    m_impl->content.workDir = workDir;
    m_impl->content.desDir = desDir;
}
FepTcpSocket::~FepTcpSocket()
{
    stopFep();
    delete m_impl;
}
void FepTcpSocket::stopFep()
{
    if (isRunning())
    {
        this->requestInterruption();
        quit();
        wait();
    }
}
bool FepTcpSocket::startFep(qintptr handle, const QMap<QString, QVariantMap>& ipCheckData)
{
    if (isRunning())
    {
        return false;
    }
    m_impl->ipCheckData = ipCheckData;
    m_impl->handle = handle;

    start();
    return true;
}

bool FepTcpSocket::init(QTcpSocket& tcpLink)
{
    tcpLink.setReadBufferSize(ProtocolLen::fepPackMaxData);
    tcpLink.setSocketDescriptor(m_impl->handle);
    m_impl->handle = qintptr();
    m_impl->content.srcIP = tcpLink.peerAddress().toString();
    m_impl->content.srcPort = tcpLink.peerPort();

    /* 20210818 wp??
     * 这里做一个检查
     * 断开非法IP
     */
    if (!m_impl->ipCheckData.isEmpty())
    {
        /* 非法IP,直接干掉就行 */
        if (!m_impl->ipCheckData.contains(m_impl->content.srcIP))
        {
            return false;
        }
        m_impl->userData = m_impl->ipCheckData.value(m_impl->content.srcIP);
    }
    else
    {
        m_impl->userData["ip"] = (m_impl->content.srcIP);
        m_impl->userData["port"] = (m_impl->content.srcPort);
    }

    pushInfoLog(QString("Fep模块,中心发送连接成功, ip:%1 port:%2").arg(m_impl->content.srcIP).arg(m_impl->content.srcPort));

    QString ip, port;

    if (m_impl->userData.contains("ip"))
    {
        ip = m_impl->userData["ip"].toString();
    }
    if (m_impl->userData.contains("port"))
    {
        port = m_impl->userData["port"].toString();
    }
    pushInfoLog(QString("Fep模块,应答 ip:%1 port:%2").arg(ip, port));
    return true;
}
bool FepTcpSocket::recvDataHandle(QTcpSocket& tcpLink)
{
    for (;;)
    {
        auto packSize = m_impl->buffer.size();
        if (packSize <= 0)
        {
            break;
        }
        /* 第一个字节是类型 */
        auto type = FepPackType(m_impl->buffer.at(0));
        switch (type)
        {
        case FepPackType::SendREQ: /* 发送请求包 */
        {
            static const auto packLen = ProtocolLen::fepPackSendREQ;
            if (packSize < packLen)
            {
                /* 长度不够再等等 */
                break;
            }

            auto byteArray = m_impl->buffer.left(packLen);
            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(packLen);
            return recvSendREQHandle(tcpLink, byteArray);
        }
        case FepPackType::Data: /* 数据包 */
        {
            /* 数据长度是动态的,这个长度是最小的长度 */
            if (packSize < ProtocolLen::fepPackMinData)
            {
                /* 长度不够再等等 */
                break;
            }

            auto& content = m_impl->content;
            /* 检查数据长度 */
            /* 已经接收的数据长度 */
            auto recvLen = content.recvFileSize;
            /* 文件总长 */
            auto fileLength = content.recvFileInfo.fileLength;
            /* 当前应该接收的数据包大小 */
            auto curRecvDataLen = fileLength > recvLen + ProtocolLen::fepDataMaxLen ? ProtocolLen::fepDataMaxLen : fileLength - recvLen;
            /* 数据长度不够,那等会再接收 */
            if (static_cast<quint32>(m_impl->buffer.size()) - ProtocolLen::fepPackMinData < curRecvDataLen)
            {
                break;
            }
            auto byteArray = m_impl->buffer.left(ProtocolLen::fepPackMinData + curRecvDataLen);
            /* 删除已经读取的 */
            m_impl->buffer = m_impl->buffer.mid(ProtocolLen::fepPackMinData + curRecvDataLen);

            return recvFepDataHandle(tcpLink, byteArray);
        }
        case FepPackType::AnserREQ: /* 请求应答包 */
        case FepPackType::Finish:   /* 结束确认包 */
        {
            break;
        }
        }

        /* 不该来的来了 */
        stateSwitch(FepRecvState::Abort, QString("Fep数据解析错误:%1").arg(QString(m_impl->buffer.toHex())));
        return false;
    }

    return true;
}
void FepTcpSocket::run()
{
    /* 创建Socket 必须在线程中创建 */
    QTcpSocket tcpLink; /* 链接的socket */
    if (!init(tcpLink))
    {
        return;
    }

    m_impl->buffer.clear();
    int waitCount = 0; /* 等待计数 一段时间后无响应会退出线程 */
    int waitIntervalTimeMS = 5;
    int waitTimeoutCount = 1000 / waitIntervalTimeMS * 5;
    while (!isInterruptionRequested())
    {
        QApplication::processEvents();
        QThread::msleep(waitIntervalTimeMS);
        auto tempBuffer = tcpLink.readAll();
        /* 5秒无数据断开连接 */
        if (waitCount > waitTimeoutCount)
        {
            break;
        }
        if (tempBuffer.isEmpty())
        {
            ++waitCount;
            continue;
        }
        else
        {
            waitCount = 0;
        }

        /* 有数据了 */
        m_impl->buffer.append(tempBuffer);

        if (!recvDataHandle(tcpLink))
        {
            break;
        }
    }

    tcpLink.disconnectFromHost();
    if (tcpLink.state() != QTcpSocket::SocketState::UnconnectedState && !tcpLink.waitForDisconnected(10000))
    {
        tcpLink.abort();
    }
}
bool FepTcpSocket::sendData(QTcpSocket& tcpLink, const QByteArray& sendData)
{
    auto ret = tcpLink.write(sendData);
    if (ret < 0 || !tcpLink.waitForBytesWritten())
    {
        stateSwitch(FepRecvState::Abort, "Fep:写入数据错误");
        return false;
    }

    return true;
}

void FepTcpSocket::stateSwitch(FepRecvState state, const QString& msgError)
{
    switch (state)
    {
    case FepRecvState::FullFile: /* 完整的文件 */
    {
        /* 发送 发送文件的请求 */
        auto bak1 = m_impl->content.workDir;
        auto bak2 = m_impl->content.desDir;
        m_impl->content.reset();
        m_impl->content.workDir = bak1;
        m_impl->content.desDir = bak2;
        break;
    }
    case FepRecvState::Wait: /* 等待文件接收 */
    {
        auto bak1 = m_impl->content.workDir;
        auto bak2 = m_impl->content.desDir;
        m_impl->userData["path"] = m_impl->content.recvFileInfo.fileName;
        m_impl->content.reset();
        m_impl->content.workDir = bak1;
        m_impl->content.desDir = bak2;
        emit sg_oneFileRecvSuccess(m_impl->userData);
        break;
    }
    case FepRecvState::Abort: /* 异常结束 */
    case FepRecvState::Fault: /* 接收故障 */
    {
        requestInterruption();
        pushErrorLog(msgError);
        break;
    }
    }
}

void FepTcpSocket::pushInfoLog(const QString& logStr) { SystemLogPublish::infoMsg(logStr); }
void FepTcpSocket::pushWarningLog(const QString& logStr) { SystemLogPublish::warningMsg(logStr); }
void FepTcpSocket::pushErrorLog(const QString& logStr) { SystemLogPublish::errorMsg(logStr); }
void FepTcpSocket::pushSpecificTipLog(const QString& logStr) { SystemLogPublish::specificTipsMsg(logStr); }

bool FepTcpSocket::fileExistsOpt(QTcpSocket& tcpLink, QFileInfo& info, const FepSendREQ& pack)
{
    auto& content = m_impl->content;
    auto& recvFileInfo = content.recvFileInfo;
    /* 认为是断点续传 */
    auto curFileSize = info.size();

    qint32 unitNum = 0;
    /* 未传完的文件居然比原文件大肯定不是同一个文件,删除重来 */
    if (curFileSize > pack.fileLen)
    {
        if (!QFile::remove(recvFileInfo.absoluteFilePath))
        {
            stateSwitch(FepRecvState::Fault, QString("FEP:本地文件%1已存在,但是文件无法操作").arg(recvFileInfo.fileName));
            return false;
        }
    }

    /*
     * 相等认为是同一个文件
     * 这里根据协议没有做很严格的判断,直接比对文件大小相等就认为是同一个文件了
     * 协议中只能根据文件名与文件大小来做模糊判断
     */
    else if (curFileSize == pack.fileLen)
    {
        pushInfoLog(QString("FEP:%1文件已存在,接收下一个文件").arg(recvFileInfo.fileName));
        /* 文件已经存在 */
        unitNum = -1;
    }
    else
    {
        pushInfoLog(QString("FEP:%1断点续传").arg(recvFileInfo.fileName));
        /* 不是发送单元的整倍数,认为文件错误删掉重传 */
        if (curFileSize % ProtocolLen::fepDataMaxLen != 0)
        {
            if (!QFile::remove(recvFileInfo.absoluteFilePath))
            {
                stateSwitch(FepRecvState::Fault, QString("FEP:本地文件%1已存在,但是文件无法操作").arg(recvFileInfo.fileName));
                return false;
            }
        }

        /* 条件符合了,认为是同一个文件,开始计算单元号 */
        unitNum = curFileSize / ProtocolLen::fepDataMaxLen;
    }

    FepAnserREQ anserPack;
    anserPack.fileID = FepTcpSocketImpl::createFileID(recvFileInfo.absoluteFilePath);
    anserPack.unitNum = unitNum;
    anserPack.fileName = pack.fileName;

    content.curUnitNum = unitNum;
    content.recvFileSize = static_cast<quint32>(unitNum) * static_cast<quint32>(ProtocolLen::fepDataMaxLen);

    QString errMsg;
    auto sendByte = FepProtocolSerialize::toByteArray(errMsg, anserPack);
    if (sendByte.isEmpty())
    {
        stateSwitch(FepRecvState::Abort, errMsg);
        return false;
    }
    /* 完整的文件,下一个 */
    if (unitNum < 0)
    {
        stateSwitch(FepRecvState::FullFile);
    }

    return sendData(tcpLink, sendByte);
}

bool FepTcpSocket::recvSendREQHandle(QTcpSocket& tcpLink, const QByteArray& rawData)
{
    FepSendREQ pack;
    QString errMsg;
    if (!FepProtocolSerialize::fromByteArray(errMsg, pack, rawData))
    {
        stateSwitch(FepRecvState::Abort, errMsg);
        return false;
    }
    if (pack.fileLen == 0 || pack.fileName.isEmpty())
    {
        stateSwitch(FepRecvState::Abort, "传输文件错误无文件名或无大小");
        return false;
    }
    auto msg = QString("fep收到文件：%1 文件大小：%2字节").arg(pack.fileName).arg(pack.fileLen);
    pushSpecificTipLog(msg);

    auto& content = m_impl->content;
    auto& recvFileInfo = content.recvFileInfo;
    recvFileInfo.fileName = pack.fileName;
    recvFileInfo.fileLength = pack.fileLen;

    QDir dir(content.workDir);
    recvFileInfo.absoluteFilePath = dir.filePath(pack.fileName);

    QFileInfo info(content.recvFileInfo.absoluteFilePath);

    /*
     * 文件不存在直接发送就ok
     * 文件存在
     *  文件比客户端发送的文件小且文件大小能被最大单次数据大小整除 断点重传
     *  文件比客户端发送的文件小且文件大小能被不能最大单次数据大小整除 删除重传
     */
    if (info.exists())
    {
        return fileExistsOpt(tcpLink, info, pack);
    }

    /* 文件不存在 */
    FepAnserREQ anserPack;
    anserPack.fileID = FepTcpSocketImpl::createFileID(recvFileInfo.absoluteFilePath);
    anserPack.unitNum = 0;
    anserPack.fileName = pack.fileName;
    content.curUnitNum = 0;
    content.recvFileSize = 0;

    auto sendByte = FepProtocolSerialize::toByteArray(errMsg, anserPack);
    if (sendByte.isEmpty())
    {
        stateSwitch(FepRecvState::Abort, errMsg);
        return false;
    }

    return sendData(tcpLink, sendByte);
}

bool FepTcpSocket::recvFepDataHandle(QTcpSocket& tcpLink, const QByteArray& rawData)
{
    auto& content = m_impl->content;
    auto& recvFileInfo = content.recvFileInfo;

    FepData pack;
    QString errMsg;
    if (!FepProtocolSerialize::fromByteArray(errMsg, pack, rawData))
    {
        stateSwitch(FepRecvState::Abort, errMsg);
        return false;
    }

    /* 每次的单元号应当递增1 如果不是则数据包错误 */
    if (content.curUnitNum != pack.unitNum)
    {
        stateSwitch(FepRecvState::Abort, "Fep:数据包错误,当前单元号与预期不符");
        return false;
    }

    /* 文件总大小 */
    auto fileLength = recvFileInfo.fileLength;
    /* 当前已接收文件的大小 */
    auto& recvLen = content.recvFileSize;
    /*
     * 根据协议当文件大小为单元大小的整倍数时会在发完数据后多发送一次字节为为0的数据包以示终止
     * 这个就是最后一次
     */
    if (recvLen == fileLength)
    {
        /* 接收完成应答 */
        if (!sendFinishPack(tcpLink))
        {
            return false;
        }
        /* 等待下一个文件 */
        stateSwitch(FepRecvState::Wait);
        return true;
    }

    if (!FepTcpSocketImpl::writeToFile(content, pack.data, errMsg))
    {
        stateSwitch(FepRecvState::Abort, errMsg);
        return false;
    }

    ++content.curUnitNum;
    recvLen += static_cast<quint32>(pack.data.size());
    pushInfoLog(QString("curUnitNum:%1 recvLen %2").arg(content.curUnitNum).arg(recvLen));

    /* 认为接收完了  */
    if (recvLen == fileLength)
    {
        /* 接收完成应答 */
        if (!sendFinishPack(tcpLink))
        {
            return false;
        }
        /* 这里多处理一下 将接收的文件移动到目标目录 */
        QDir dir(m_impl->content.workDir);
        auto absoluteFilePath1 = dir.filePath(m_impl->content.recvFileInfo.fileName);

        dir = QDir(m_impl->content.desDir);
        auto absoluteFilePath2 = dir.filePath(m_impl->content.recvFileInfo.fileName);
        QFile::copy(absoluteFilePath1, absoluteFilePath2);

        stateSwitch(FepRecvState::Wait);
    }

    return true;
}

bool FepTcpSocket::sendFinishPack(QTcpSocket& tcpLink)
{
    FepFinish pack;
    pack.fileID = m_impl->content.recvFileInfo.fileID;

    QString errMsg;
    auto sendArray = FepProtocolSerialize::toByteArray(errMsg, pack);
    if (sendArray.isEmpty())
    {
        stateSwitch(FepRecvState::Abort, errMsg);
        return false;
    }

    pushInfoLog(QString("文件接收完成响应:%1").arg(QString(sendArray.toHex())));
    return sendData(tcpLink, sendArray);
}
