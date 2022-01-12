#include "NetInterfaceHelper.h"

NetInterfaceHelper NetInterfaceHelper::m_instance;

NetInterfaceHelper& NetInterfaceHelper::instance() { return m_instance; }
NetInterfaceHelper::~NetInterfaceHelper() {}
QList<QNetworkInterface> NetInterfaceHelper::ipMapToNetInterfaceList(const QString& localIP)
{
    /* 已经用过这个IP了 */
    auto find = m_interfaceMap.find(localIP);
    if (find != m_interfaceMap.end())
    {
        return find.value();
    }
    /* 没有找到 */
    QList<QNetworkInterface> list;
    auto netInterfaces = QNetworkInterface::allInterfaces();
    for (auto& netInterface : netInterfaces)
    {
        auto entryList = netInterface.addressEntries();
        //遍历每一个IP地址(每个包含一个IP地址，一个子网掩码和一个广播地址)
        for (auto& entry : entryList)
        {
            // 广播地址
            auto broadcastAddress = entry.broadcast();
            if (entry.ip().toString() != localIP || broadcastAddress == QHostAddress::Null ||
                entry.ip().protocol() != QAbstractSocket::IPv4Protocol || entry.ip() == QHostAddress::LocalHost)
            {
                continue;
            }
            list << netInterface;
            break;
        }
    }
    m_interfaceMap.insert(localIP, list);
    return list;
}
NetInterfaceHelper::NetInterfaceHelper() {}
