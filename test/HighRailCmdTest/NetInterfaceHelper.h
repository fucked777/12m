#ifndef NETINTERFACEHELPER_H
#define NETINTERFACEHELPER_H
#include <QNetworkInterface>
#include <QString>
class NetInterfaceHelper
{
public:
    static NetInterfaceHelper& instance();
    ~NetInterfaceHelper();
    QList<QNetworkInterface> ipMapToNetInterfaceList(const QString& localIP);

private:
    NetInterfaceHelper(const NetInterfaceHelper&) = delete;
    NetInterfaceHelper& operator=(const NetInterfaceHelper&) = delete;
    NetInterfaceHelper(NetInterfaceHelper&&) = delete;
    NetInterfaceHelper& operator=(NetInterfaceHelper&&) = delete;
    NetInterfaceHelper();

private:
    static NetInterfaceHelper m_instance;
    QMap<QString, QList<QNetworkInterface>> m_interfaceMap;
};

#endif  // NETINTERFACEHELPER_H
