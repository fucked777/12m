#ifndef DeviceStatusLogSERVICE_H
#define DeviceStatusLogSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class DeviceStatusLogServiceImpl;
class DeviceStatusLogService : public INetMsg
{
    Q_OBJECT
public:
    explicit DeviceStatusLogService(cppmicroservices::BundleContext context);
    ~DeviceStatusLogService();

private:
    void searchACK(const ProtocolPack& pack);

signals:
    void signalSearchACK(const QByteArray& data);

public slots:
    void search(const QByteArray& data);

private:
    DeviceStatusLogServiceImpl* m_impl;
};
#endif  // DeviceStatusLogSERVICE_H
