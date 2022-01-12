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
public:
    DeviceStatusLogService(cppmicroservices::BundleContext context);
    ~DeviceStatusLogService();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void start();
    void stop();
    void addItem(const ProtocolPack& pack);
    void deleteItem(const ProtocolPack& pack);
    void queryItem(const ProtocolPack& pack);
    void modifyItem(const ProtocolPack& pack);
    void userLogin(const ProtocolPack& pack);

private:
    DeviceStatusLogServiceImpl* mImpl;
};

#endif  // DeviceStatusLogSERVICE_H
