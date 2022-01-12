#include "UdpChannelP2P.h"
#include "BaseChannelImpl.h"
#include "DevProtocolSerialize.h"
#include "NetInterfaceHelper.h"
#include "Utility.h"
#include <QHostAddress>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
#include <QNetworkDatagram>
#endif

class UdpChannelP2PImpl
{
    DIS_COPY_MOVE(UdpChannelP2PImpl)
public:
    inline UdpChannelP2PImpl() {}
    inline ~UdpChannelP2PImpl() {}
    QUdpSocket* socket{ nullptr };
};

UdpChannelP2P::UdpChannelP2P(const CommunicationInfo& info, QObject* parent)
    : BaseChannel(info, parent)
    , m_impl(new UdpChannelP2PImpl)
{
}

UdpChannelP2P::~UdpChannelP2P()
{
    unInitRecvSocket();
    unInitSendSocket();
    delete m_impl;
    m_impl = nullptr;
}

OptionalNotValue UdpChannelP2P::sendData(const QByteArray& byteArray)
{
    m_baseImpl->sendStatus.data = byteArray;
    /* 无效的socket 直接返回 */
    if (!sendVaild())
    {
        m_baseImpl->sendStatus.state = QAbstractSocket::UnconnectedState;
        ++m_baseImpl->sendStatus.failCount;
        emit sg_channelSendDataChange(m_baseImpl->sendStatus);
        return OptionalNotValue(ErrorCode::NotInit, "发送通道未初始化");
    }
    auto& socket = m_impl->socket;
    m_baseImpl->sendStatus.state = socket->state();
    auto sendRet = socket->writeDatagram(byteArray, QHostAddress(m_baseImpl->channelInfo.desIP), m_baseImpl->channelInfo.sendPort);

    if (sendRet < 0)
    {
        ++m_baseImpl->sendStatus.failCount;
        emit sg_channelSendDataChange(m_baseImpl->sendStatus);
        //发送数据失败
        return OptionalNotValue(ErrorCode::DataSendFailed, socket->errorString());
    }

    ++m_baseImpl->sendStatus.count;
    emit sg_channelSendDataChange(m_baseImpl->sendStatus);
    return OptionalNotValue();
}

bool UdpChannelP2P::recvVaild() { return m_impl->socket != nullptr; }
bool UdpChannelP2P::sendVaild() { return m_impl->socket != nullptr; }
bool UdpChannelP2P::initSendSocket()
{
    //增大缓存区大小
    static constexpr int bufferSize = 128 * 1024 * 1024;
    if (sendVaild())
    {
        return true;
    }

    auto& info = m_baseImpl->channelInfo;
    auto socket = new QUdpSocket;
    GetValueHelper<QUdpSocket> helper(socket);

    socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
    socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, bufferSize);

    /* 禁用自回环 */
    socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
    /* 通过UDP通讯时要过两个交换机，如果你不进行TTL设置，在第二台交换机就已经收不到你的UDP报文，导致你发的报文中途丢掉了
     * 所以要进行TTL设置(就像游戏中你有255条命一样)
     */
    socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 0XFF);

    if (!socket->bind(QHostAddress(info.localIP), info.recvPort, QAbstractSocket::ReuseAddressHint))
    {
        qWarning() << QString("Udp(点对点) 绑定失败,ID:%1  错误信息: %2").arg(info.id, 0, 16).arg(socket->errorString());

        return false;
    }

    connect(socket, &QIODevice::readyRead, this, &UdpChannelP2P::onReadyRead);
    m_impl->socket = helper.get();
    return true;
}
bool UdpChannelP2P::initRecvSocket() { return true; }
void UdpChannelP2P::unInitSendSocket()
{
    if (!sendVaild())
    {
        return;
    }
    m_impl->socket->flush();
    m_impl->socket->close();
    delete m_impl->socket;
    m_impl->socket = nullptr;
    BaseChannel::unInitSendSocket();
}
void UdpChannelP2P::unInitRecvSocket() { BaseChannel::unInitRecvSocket(); }
void UdpChannelP2P::onReadyRead()
{
    auto& socket = m_impl->socket;
    if (!socket->hasPendingDatagrams())
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    QNetworkDatagram datagram = socket->receiveDatagram();
    auto data = datagram.data();
#else
    auto count = socket->pendingDatagramSize();
    QByteArray data;
    data.resize(count);
    socket->readDatagram(data.data(), count);
#endif

// 网络部分不去管协议了 20211027
#if 0
    ++m_baseImpl->recvStatus.count;
    emit sg_channelRecvDataChange(m_baseImpl->recvStatus);

    /* 发送数据包 */
    emit sg_recvPack(m_baseImpl->channelInfo, data);
    return;
#else
    auto& info = m_baseImpl->channelInfo;

    m_baseImpl->recvStatus.data = data;
    m_baseImpl->recvStatus.state = socket->state();

    /* 20210804 wp?? 新增不检查协议 */
    if (!info.devProtocolCheck)
    {
        ++m_baseImpl->recvStatus.count;
        emit sg_channelRecvDataChange(m_baseImpl->recvStatus);

        /* 发送数据包 */
        emit sg_recvPack(m_baseImpl->channelInfo, data);
        return;
    }

    if (data.size() < DevProtocolLength::value)
    {
        ++m_baseImpl->recvStatus.failCount;
        auto errMsg = QString("报文长度太短");
        emit sg_recvErrorPack(data, errMsg);
        emit sg_channelRecvDataChange(m_baseImpl->recvStatus);
        return;
    }

    /* 协议头单独拿出来 */
    auto headByte = data.left(DevProtocolLength::value);
    auto real = data.mid(DevProtocolLength::value);

    /* 解协议头 */
    DevProtocolHeader head;
    headByte >> head;
    /* 协议解析失败 */
    if (!head.isValid())
    {
        ++m_baseImpl->recvStatus.failCount;
        auto errMsg = QString("协议头解析错误");
        emit sg_recvErrorPack(data, errMsg);
        emit sg_channelRecvDataChange(m_baseImpl->recvStatus);
        return;
    }
    /* 数据超过这个最大大小,认为乱包了 */
    if (head.dataLen > m_baseImpl->packMax)
    {
        ++m_baseImpl->recvStatus.failCount;
        auto errMsg = QString("数据长度超出最大限制长度");
        emit sg_recvErrorPack(data, errMsg);
        emit sg_channelRecvDataChange(m_baseImpl->recvStatus);
        return;
    }
    /* 协议头解析结束 */

    /* 数据 */
    /* 实际数据与协议头对不上 */
    if (real.size() != head.dataLen)
    {
        ++m_baseImpl->recvStatus.failCount;
        auto errMsg = QString("实际长度与协议头不对应");
        emit sg_recvErrorPack(data, errMsg);
        emit sg_channelRecvDataChange(m_baseImpl->recvStatus);
        return;
    }

    ++m_baseImpl->recvStatus.count;
    emit sg_channelRecvDataChange(m_baseImpl->recvStatus);

    if (!info.onlyRecvTargetData)
    {
        /* 发送数据包 */
        emit sg_recvPack(m_baseImpl->channelInfo, data);
        return;
    }

    /* 看看是不是发送给自己的 */
    if (head.did == m_baseImpl->compareHead.did)
    {
        /* 发送数据包 */
        emit sg_recvPack(m_baseImpl->channelInfo, data);
    }
#endif
}
