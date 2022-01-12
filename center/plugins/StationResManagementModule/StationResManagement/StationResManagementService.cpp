#include "StationResManagementService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "StationResManagement";
class StationResManagementServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    /*
     * 组向服务器设备管理的包
     */
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
StationResManagementService::StationResManagementService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new StationResManagementServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addItemACK", &StationResManagementService::addItemACK, this);
    registerSubscribe("deleteItemACK", &StationResManagementService::deleteItemACK, this);
    registerSubscribe("modifyItemACK", &StationResManagementService::modifyItemACK, this);
}

StationResManagementService::~StationResManagementService() { delete m_impl; }

void StationResManagementService::addItem(const QByteArray& data)
{
    auto pack = StationResManagementServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}
void StationResManagementService::deleteItem(const QByteArray& data)
{
    auto pack = StationResManagementServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}
void StationResManagementService::modifyItem(const QByteArray& data)
{
    auto pack = StationResManagementServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}

void StationResManagementService::addItemACK(const ProtocolPack& pack) { emit sg_addACK(pack.data); }
void StationResManagementService::deleteItemACK(const ProtocolPack& pack) { emit sg_deleteACK(pack.data); }
void StationResManagementService::modifyItemACK(const ProtocolPack& pack) { emit sg_modifyACK(pack.data); }
