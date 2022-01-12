#include "DeviceStatusLogService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "ServerStatusPersistenceService";

class DeviceStatusLogServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    static inline ProtocolPack packServerSelf(const char* operation, const char* operationACK = "", const QByteArray& data = QByteArray())
    {
        ProtocolPack sendPack;
        sendPack.desID = serverSelfModule;
        sendPack.operation = operation;
        sendPack.operationACK = operationACK;
        sendPack.module = false;
        sendPack.data = data;
        return sendPack;
    }
};
DeviceStatusLogService::DeviceStatusLogService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new DeviceStatusLogServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("searchACK", &DeviceStatusLogService::searchACK, this);
}

DeviceStatusLogService::~DeviceStatusLogService() { delete m_impl; }

void DeviceStatusLogService::searchACK(const ProtocolPack& pack) { emit signalSearchACK(pack.data); }

void DeviceStatusLogService::search(const QByteArray& data)
{
    auto pack = DeviceStatusLogServiceImpl::packServerSelf("search", "searchACK", data);
    silenceSend(pack);
}
