#include "TimeSyncUDPGB.h"
#include "GlobalData.h"
#include "NTPTime.h"
#include "NetInterfaceHelper.h"
#include "PlatformInterface.h"
#include "TimeMessageDefine.h"
#include "TimeMessageSerialize.h"

#include <QDateTime>
#include <QSettings>
#include <QUdpSocket>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
#include <QNetworkDatagram>
#endif
class TimeSyncUDPGBImpl
{
public:
    QUdpSocket* socket{ nullptr };

    QString localIP;        /* 本地IP */
    quint16 localPort{ 0 }; /* 本地端口 */

    QString desIP;        /* 目标IP地址 */
    quint16 desPort{ 0 }; /* 目标端口 */

    void readConfig();
};

void TimeSyncUDPGBImpl::readConfig()
{
    auto timeConfigPath = PlatformConfigTools::configBusiness("TimeConfig/TimeNetConfig.ini");
    QSettings setting(timeConfigPath, QSettings::IniFormat);

    localIP = setting.value("Net/LocalIP", "127.0.0.1").toString();
    localPort = static_cast<quint16>(setting.value("Net/LocalPort", 20000).toUInt());

    desIP = setting.value("Net/DesIP", "127.0.0.1").toString();
    desPort = static_cast<quint16>(setting.value("Net/DesPort", 20001).toUInt());
}

TimeSyncUDPGB::TimeSyncUDPGB()
    : m_impl(new TimeSyncUDPGBImpl)
{
    connect(this, &TimeSyncUDPGB::sg_setLocalTimeData, this, &TimeSyncUDPGB::setLocalTimeData);
}

TimeSyncUDPGB::~TimeSyncUDPGB()
{
    if (m_impl->socket != nullptr)
    {
        m_impl->socket->close();
        delete m_impl->socket;
        m_impl->socket = nullptr;
    }
    delete m_impl;
}

OptionalNotValue TimeSyncUDPGB::initSocket()
{
    //增大缓存区大小
    static constexpr int bufferSize = 128 * 1024 * 1024;
    if (m_impl->socket != nullptr)
    {
        return OptionalNotValue();
    }

    m_impl->readConfig();
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

    auto result = socket->bind(QHostAddress::AnyIPv4, m_impl->localPort, QAbstractSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!result)
    {
        auto msg = QString("时间模块:Udp(组播) 绑定失败,错误信息: %1").arg(socket->errorString());
        return OptionalNotValue(ErrorCode::UDPInitFailed, msg);
    }

    auto netInterfaces = NetInterfaceHelper::instance().ipMapToNetInterfaceList(m_impl->localIP);

    for (auto& netInterface : netInterfaces)
    {
        result = socket->joinMulticastGroup(QHostAddress(m_impl->desIP), netInterface);
        if (!result)
        {
            auto msg = QString("时间模块:Udp(组播) 加入失败,错误信息: %1").arg(socket->errorString());
            return OptionalNotValue(ErrorCode::UDPInitFailed, msg);
        }
    }

    m_impl->socket = helper.get();
    return OptionalNotValue();
}

/* 同步时间 */
bool TimeSyncUDPGB::syncTime(const TimeData& timeData)
{
    if (m_impl->socket == nullptr)
    {
        return false;
    }
    // 服务器保存时间信息
    emit sg_setLocalTimeData(timeData);

    QByteArray json;
    json << timeData;

    auto sendRet = m_impl->socket->writeDatagram(json, QHostAddress(m_impl->desIP), m_impl->desPort);
    return sendRet >= 0;
}
void TimeSyncUDPGB::setLocalTimeData(const TimeData& timeData) { GlobalData::setTimeData(timeData); }
