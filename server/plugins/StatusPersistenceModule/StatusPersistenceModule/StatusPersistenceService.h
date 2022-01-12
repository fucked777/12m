#ifndef STATUSPERSISTENCESERVICE_H
#define STATUSPERSISTENCESERVICE_H

#include "CustomPacketMessageDefine.h"
#include "INetMsg.h"
#include "RedisHelper.h"

namespace cppmicroservices
{
    class BundleContext;
}

class StatusPersistenceServiceImpl;
class StatusTcpServer;
class StatusSqlQuery;
class AutoTestDataTcpServer;
class StatusPersistenceService : public INetMsg
{
public:
    StatusPersistenceService(cppmicroservices::BundleContext context);
    ~StatusPersistenceService();

    //void search(const ProtocolPack& pack);

private:
    void masterSlave(const HeartbeatMessage& msg);

private:
    StatusPersistenceServiceImpl* m_impl;
    StatusTcpServer* mStatusTcpServer = nullptr;
    StatusSqlQuery* m_statusSqlQuery = nullptr;
    AutoTestDataTcpServer* mAutoTestDataTcpServer = nullptr;
    cpp_redis::subscriber logSuber;
};
#endif  // STATUSPERSISTENCESERVICE_H
