#include "UdpChannelMulticast.h"
#include "BaseChannelImpl.h"
#include "DevProtocol.h"
#include "DevProtocolSerialize.h"
#include "ExtendedConfig.h"
#include "NetInterfaceHelper.h"
#include "Utility.h"
#include <QHostAddress>
#include <QNetworkInterface>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
#include <QNetworkDatagram>
#endif

class UdpChannelMulticastImpl
{
    DIS_COPY_MOVE(UdpChannelMulticastImpl)
public:
    inline UdpChannelMulticastImpl() {}
    inline ~UdpChannelMulticastImpl() {}
    QUdpSocket* sendSocket{ nullptr };
    QUdpSocket* recvSocket{ nullptr };
};

UdpChannelMulticast::UdpChannelMulticast(const CommunicationInfo& info, QObject* parent)
    : BaseChannel(info, parent)
    , m_impl(new UdpChannelMulticastImpl)
{
}

UdpChannelMulticast::~UdpChannelMulticast() { delete m_impl; }
bool UdpChannelMulticast::recvVaild() { return m_impl->recvSocket != nullptr; }
bool UdpChannelMulticast::sendVaild() { return m_impl->sendSocket != nullptr; }
OptionalNotValue UdpChannelMulticast::sendData(const QByteArray& byteArray)
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
    auto& socket = m_impl->sendSocket;
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
bool UdpChannelMulticast::initSendSocket()
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

    //增大发送和接收缓存区大小
    // socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
    socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, bufferSize);
    /* 禁用自回环 */
    socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
    /* 通过UDP通讯时要过两个交换机，如果你不进行TTL设置，在第二台交换机就已经收不到你的UDP报文，导致你发的报文中途丢掉了
     * 所以要进行TTL设置(就像游戏中你有255条命一样)
     */
    socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 0XFF);

    auto result = socket->bind(QHostAddress::AnyIPv4, info.sendPort, QAbstractSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!result)
    {
        qWarning() << QString("Udp(组播) 绑定失败,ID:%1  错误信息: %2").arg(info.id, 0, 16).arg(socket->errorString());
        return false;
    }

    auto netInterfaces = NetInterfaceHelper::instance().ipMapToNetInterfaceList(info.localIP);

    for (auto& netInterface : netInterfaces)
    {
        result = socket->joinMulticastGroup(QHostAddress(info.desIP), netInterface);
        if (!result)
        {
            qWarning() << QString("Udp(组播) 加入失败,ID:%1 错误信息: %2").arg(info.id, 0, 16).arg(socket->errorString());

            return false;
        }
    }

    // connect(socket, &QIODevice::readyRead, this, &UdpChannelMulticast::onReadyRead);
    m_impl->sendSocket = helper.get();
    return true;
}
bool UdpChannelMulticast::initRecvSocket()
{
    //增大缓存区大小
    static constexpr int bufferSize = 128 * 1024 * 1024;
    if (recvVaild())
    {
        return true;
    }

    auto& info = m_baseImpl->channelInfo;
    auto socket = new QUdpSocket;
    GetValueHelper<QUdpSocket> helper(socket);

    //增大发送和接收缓存区大小
    socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
    // socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, bufferSize);
    /* 禁用自回环 */
    socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
    /* 通过UDP通讯时要过两个交换机，如果你不进行TTL设置，在第二台交换机就已经收不到你的UDP报文，导致你发的报文中途丢掉了
     * 所以要进行TTL设置(就像游戏中你有255条命一样)
     */
    socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 0XFF);

    auto result = socket->bind(QHostAddress::AnyIPv4, info.recvPort, QAbstractSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!result)
    {
        qWarning() << QString("Udp(组播) 绑定失败,ID:%1  错误信息: %2").arg(info.id, 0, 16).arg(socket->errorString());

        return false;
    }

    auto netInterfaces = NetInterfaceHelper::instance().ipMapToNetInterfaceList(info.localIP);

    for (auto& netInterface : netInterfaces)
    {
        result = socket->joinMulticastGroup(QHostAddress(info.desIP), netInterface);
        if (!result)
        {
            qWarning() << QString("Udp(组播) 加入失败,ID:%1 错误信息: %2").arg(info.id, 0, 16).arg(socket->errorString());

            return false;
        }
    }

    connect(socket, &QIODevice::readyRead, this, &UdpChannelMulticast::onReadyRead);
    m_impl->recvSocket = helper.get();
    return true;
}
void UdpChannelMulticast::unInitSendSocket()
{
    if (!sendVaild())
    {
        return;
    }
    m_impl->sendSocket->flush();
    m_impl->sendSocket->close();
    delete m_impl->sendSocket;
    m_impl->sendSocket = nullptr;
    BaseChannel::unInitSendSocket();
}

void UdpChannelMulticast::unInitRecvSocket()
{
    if (!recvVaild())
    {
        return;
    }
    m_impl->recvSocket->close();
    delete m_impl->recvSocket;
    m_impl->recvSocket = nullptr;
    BaseChannel::unInitRecvSocket();
}

void UdpChannelMulticast::onReadyRead()
{
    auto& socket = m_impl->recvSocket;
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
    m_baseImpl->recvStatus.data = data;
    m_baseImpl->recvStatus.state = socket->state();

    ++m_baseImpl->recvStatus.count;
    emit sg_channelRecvDataChange(m_baseImpl->recvStatus);

    /* 发送数据包 */
    emit sg_recvPack(m_baseImpl->channelInfo, data);

    return;

#else

    m_baseImpl->recvStatus.data = data;
    m_baseImpl->recvStatus.state = socket->state();

    auto& info = m_baseImpl->channelInfo;
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
