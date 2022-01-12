#include "FepSendUnit.h"
#include "FepProtocolSerialize.h"
#include "MessagePublish.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
    FepSendContent content;
    bool isRun{ false };

public:
    static bool setFile(FepSendContent& content, const QString& filePath, QString& errMsg);
    static bool getSendFileList(FepSendContent& content, QString& errMsg);
    static bool getTheSendFile(FepSendContent& content, FepSendFileInfo& needSendFile);
    static bool readFileContent(FepSendFileInfo& fileInfo, QByteArray& fileContent, quint32& length, QString& errMsg);
};

bool FepSendUnitImpl::setFile(FepSendContent& content, const QString& filePath, QString& errMsg)
{
    content.fileList.clear();

    QFileInfo info(filePath);
    if (!info.isFile())
    {
        errMsg = "Fep:文件不存在";
        return false;
    }
    FepSendFileInfo tmpFileInfo;
    tmpFileInfo.fileName = info.fileName();
    tmpFileInfo.absoluteFilePath = info.absoluteFilePath();
    tmpFileInfo.length = info.size();
    content.fileList.append(tmpFileInfo);
    auto dir = info.absoluteDir();
    content.workDir = dir.absolutePath();

    return true;
}
bool FepSendUnitImpl::getSendFileList(FepSendContent& content, QString& errMsg)
{
    content.fileList.clear();
    if (content.workDir.isEmpty())
    {
        errMsg = "Fep:目录路径为空";
        return false;
    }
    QDir dir(content.workDir);
    if (!dir.exists())
    {
        errMsg = "Fep:当前目录不在";
        return false;
    }
    content.workDir = dir.absolutePath();
    auto fileList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
    for (auto& file : fileList)
    {
        FepSendFileInfo tmpFileInfo;
        tmpFileInfo.fileName = file.fileName();
        tmpFileInfo.absoluteFilePath = file.absoluteFilePath();
        tmpFileInfo.length = file.size();
        content.fileList.append(tmpFileInfo);
    }
    if (content.fileList.isEmpty())
    {
        errMsg = "Fep:当前工作目录待发送文件为空";
        return false;
    }
    return true;
}
bool FepSendUnitImpl::getTheSendFile(FepSendContent& content, FepSendFileInfo& needSendFile)
{
    if (!content.fileList.isEmpty())
    {
        needSendFile = content.fileList.first();
        return true;
    }

    return false;
}
bool FepSendUnitImpl::readFileContent(FepSendFileInfo& fileInfo, QByteArray& fileContent, quint32& length, QString& errMsg)
{
    QFile tmpFile(fileInfo.absoluteFilePath);
    if (!tmpFile.open(QIODevice::ReadOnly))
    {
        errMsg = QString("Fep:文件不存在 %1").arg(fileInfo.absoluteFilePath);
        return false;
    }

    length = 0;
    fileContent.clear();

    if (fileInfo.sendLength < fileInfo.length)
    {
        tmpFile.seek(fileInfo.sendLength);
        fileContent = tmpFile.read(ProtocolLen::fepDataMaxLen);
        length = fileContent.size();
        fileInfo.sendLength += length;
    }

    ++fileInfo.curUnitNum;
    return true;
}

FepSendUnit::FepSendUnit(QObject* parent)
    : QThread(parent)
    , m_impl(new FepSendUnitImpl)
{
    qRegisterMetaType<FepSendState>("FepSendState");
    qRegisterMetaType<FepSendState>("FepSendState&");
}
FepSendUnit::~FepSendUnit()
{
    stopFep();
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
bool FepSendUnit::startSendFile(const QString& filePath, QString& errMsg)
{
    auto& content = m_impl->content;
    if (m_impl->isRun)
    {
        errMsg = "Fep:发送已在运行";
        return false;
    }
    if (!ipv4Verify(content.dstIP))
    {
        errMsg = "Fep:当前IP地址非法";
        return false;
    }
    if (!FepSendUnitImpl::setFile(m_impl->content, filePath, errMsg))
    {
        return false;
    }
    if (m_impl->content.fileList.isEmpty())
    {
        errMsg = "Fep:发送文件为空";
        return false;
    }

    m_impl->isRun = true;
    start();
    return true;
}
bool FepSendUnit::startSendDir(const QString& dirPath, QString& errMsg)
{
    auto& content = m_impl->content;
    if (m_impl->isRun)
    {
        errMsg = "Fep:发送已在运行";
        return false;
    }
    if (!ipv4Verify(content.dstIP))
    {
        errMsg = "Fep:当前IP地址非法";
        return false;
    }
    content.workDir = dirPath;
    if (!FepSendUnitImpl::getSendFileList(content, errMsg))
    {
        return false;
    }
    if (content.fileList.isEmpty())
    {
        errMsg = "Fep:发送文件为空";
        return false;
    }

    m_impl->isRun = true;
    start();
    return true;
}
void FepSendUnit::pushInfoLog(const QString& logStr) { SystemLogPublish::infoMsg(logStr); }
void FepSendUnit::pushWarningLog(const QString& logStr) { SystemLogPublish::warningMsg(logStr); }
void FepSendUnit::pushErrorLog(const QString& logStr) { SystemLogPublish::errorMsg(logStr); }
void FepSendUnit::pushSpecificTipLog(const QString& logStr) { SystemLogPublish::specificTipsMsg(logStr); }

void FepSendUnit::stopFep()
{
    if (isRunning())
    {
        this->requestInterruption();
        quit();
        wait();
    }
}

//发送数据
FepSendState FepSendUnit::sendData(QTcpSocket& tcpLink, const QByteArray& sendBuffer)
{
    auto ret = tcpLink.write(sendBuffer);
    if (ret < 0 || !tcpLink.waitForBytesWritten(5000))
    {
        pushErrorLog("Fep:写入数据错误");
        return FepSendState::Abort;
    }
    return FepSendState::Success;
}

FepSendState FepSendUnit::sendREQPack(QTcpSocket& tcpLink, const FepSendFileInfo& fileInfo)
{
    FepSendREQ pack;
    pack.fileName = fileInfo.fileName;
    pack.fileLen = fileInfo.length;

    QString errMsg;
    auto sendBuffer = FepProtocolSerialize::toByteArray(errMsg, pack);

    if (sendBuffer.isEmpty())
    {
        pushErrorLog(errMsg);
        return FepSendState::Abort;
    }

    return sendData(tcpLink, sendBuffer);
}
FepSendState FepSendUnit::recvAnserREQ(QTcpSocket& tcpLink, FepSendFileInfo& fileInfo)
{
    static const auto packLen = ProtocolLen::fepPackAnserREQ;
    int waitCount = 0; /* 等待计数 一段时间后无响应会退出线程 */
    int waitIntervalTimeMS = 5;
    int waitTimeoutCount = 1000 / waitIntervalTimeMS * 5;
    QByteArray rawData;
    while (!isInterruptionRequested())
    {
        QApplication::processEvents();
        QThread::msleep(waitIntervalTimeMS);
        auto tempBuffer = tcpLink.readAll();
        /* 5秒无数据断开连接 */
        if (waitCount > waitTimeoutCount)
        {
            pushErrorLog("Fep:等待接收响应超时");
            return FepSendState::Abort;
        }
        ++waitCount;
        if (tempBuffer.isEmpty())
        {
            continue;
        }

        rawData.append(tempBuffer);
        auto packSize = rawData.size();
        if (packSize < packLen)
        {
            /* 长度不够再等等 */
            continue;
        }
        break;
    }

    FepAnserREQ pack;
    QString errMsg;
    if (!FepProtocolSerialize::fromByteArray(errMsg, pack, rawData))
    {
        pushErrorLog(errMsg);
        return FepSendState::Abort;
    }

    /* 解析成功 */
    fileInfo.curUnitNum = pack.unitNum;
    fileInfo.sendLength = 0;
    fileInfo.fileID = pack.fileID;

    if (pack.unitNum >= 0)
    {
        fileInfo.sendLength = fileInfo.curUnitNum * ProtocolLen::fepDataMaxLen;
        if (fileInfo.sendLength > fileInfo.length)
        {
            auto sendStr = QString("Fep:数据错误,本地文件大小%1,远端断点续传文件大小%2").arg(fileInfo.length, fileInfo.sendLength);
            pushErrorLog(errMsg);
            return FepSendState::Abort;
        }
        /* 文件不存在，可以进行传输 */
        /* 断点续传 */
        return FepSendState::SendFile;
    }

    if (pack.unitNum == -1)
    {
        /* 文件是完整的,下一个文件 */
        return FepSendState::Next;
    }

    if (pack.unitNum == -2)
    {
        /* 暂时故障不能接收文件 */
        pushErrorLog("服务端故障,不能接收文件");
        return FepSendState::Close;
    }

    pushErrorLog("接收数据错误,未知的信息");
    return FepSendState::Abort;
}
FepSendState FepSendUnit::recvFepFinish(QTcpSocket& tcpLink)
{
    static const auto packLen = ProtocolLen::fepPackFinish;
    int waitCount = 0; /* 等待计数 一段时间后无响应会退出线程 */
    int waitIntervalTimeMS = 5;
    int waitTimeoutCount = 1000 / waitIntervalTimeMS * 10;
    QByteArray rawData;
    while (!isInterruptionRequested())
    {
        QApplication::processEvents();
        QThread::msleep(waitIntervalTimeMS);
        auto tempBuffer = tcpLink.readAll();
        /* 5秒无数据断开连接 */
        if (waitCount > waitTimeoutCount)
        {
            pushErrorLog("Fep:等待接收响应超时");
            return FepSendState::Abort;
        }
        ++waitCount;
        if (tempBuffer.isEmpty())
        {
            continue;
        }

        rawData.append(tempBuffer);
        auto packSize = rawData.size();
        if (packSize < packLen)
        {
            /* 长度不够再等等 */
            continue;
        }
        break;
    }

    return FepSendState::Success;
}

bool FepSendUnit::sendFlow(QTcpSocket& tcpLink, FepSendFileInfo& fileInfo)
{
    /* 1.文件请求 */
    auto state = sendREQPack(tcpLink, fileInfo);
    if (state == FepSendState::Close || state == FepSendState::Abort)
    {
        return false;
    }

    state = recvAnserREQ(tcpLink, fileInfo);
    if (state == FepSendState::Close || state == FepSendState::Abort)
    {
        return false;
    }
    if (state == FepSendState::Next)
    {
        return true;
    }

    /* 发送文件 */
    state = sendDataPack(tcpLink, fileInfo);
    if (state == FepSendState::Close || state == FepSendState::Abort)
    {
        return false;
    }
    /* 等待结束应答 */
    state = recvFepFinish(tcpLink);
    if (state == FepSendState::Close || state == FepSendState::Abort)
    {
        return false;
    }

    return true;
}

FepSendState FepSendUnit::sendDataPack(QTcpSocket& tcpLink, FepSendFileInfo& fileInfo)
{
    FepData pack;
    //此处根据文件大小来做
    while (!isInterruptionRequested())
    {
        pack.unitNum = fileInfo.curUnitNum;
        pack.fileID = fileInfo.fileID;

        pack.data.clear();
        quint32 length = 0;
        QString errMsg;
        if (!FepSendUnitImpl::readFileContent(fileInfo, pack.data, length, errMsg))
        {
            pushErrorLog(errMsg);
            return FepSendState::Abort;
        }

        //        auto tttt = QString("%1 curUnitNum:%2 length:%3 sendLength:%4")
        //                        .arg(fileInfo.fileName)
        //                        .arg(fileInfo.curUnitNum)
        //                        .arg(fileInfo.length)
        //                        .arg(fileInfo.sendLength);
        //        pushInfoLog(tttt);
        auto sendBuffer = FepProtocolSerialize::toByteArray(errMsg, pack);
        if (sendBuffer.isEmpty())
        {
            pushErrorLog(errMsg);
            return FepSendState::Abort;
        }

        auto state = sendData(tcpLink, sendBuffer);
        if (state == FepSendState::Close || state == FepSendState::Abort)
        {
            return state;
        }

        if (fileInfo.sendLength >= fileInfo.length)
        {
            /* 发完了 */
            /* 这种情况要发送一个0字节 */
            if (static_cast<int>(length) == ProtocolLen::fepDataMaxLen)
            {
                pack.data = QByteArray();
                pack.unitNum = fileInfo.curUnitNum;
                ++fileInfo.curUnitNum;
                sendBuffer = FepProtocolSerialize::toByteArray(errMsg, pack);
                state = sendData(tcpLink, sendBuffer);
                if (state == FepSendState::Close || state == FepSendState::Abort)
                {
                    return state;
                }
            }

            return FepSendState::Success;
        }

        QThread::msleep(5);
        QApplication::processEvents();
    }

    return FepSendState::Abort;
}

void FepSendUnit::run()
{
    auto& content = m_impl->content;
    QTcpSocket tcpLink;
    tcpLink.setReadBufferSize(ProtocolLen::fepPackMaxData);
    tcpLink.connectToHost(content.dstIP, content.dstPort);
    if (!tcpLink.waitForConnected(5000))
    {
        m_impl->isRun = false;
        pushInfoLog(QString("Fep:连接超时 %1:%2").arg(content.dstIP).arg(content.dstPort));
        return;
    }

    while (!isInterruptionRequested())
    {
        /* 1.获取发送的文件信息 */
        FepSendFileInfo currFile;
        if (!FepSendUnitImpl::getTheSendFile(m_impl->content, currFile))
        {
            m_impl->isRun = false;
            pushInfoLog("Fep:文件发送完成");
            return;
        }
        if (!sendFlow(tcpLink, currFile))
        {
            m_impl->isRun = false;
            return;
        }
        m_impl->content.fileList.removeFirst();
    }

    m_impl->isRun = false;
}
