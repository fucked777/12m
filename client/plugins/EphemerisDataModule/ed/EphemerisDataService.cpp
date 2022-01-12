#include "EphemerisDataService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "EphemerisData";
class EphemerisDataServiceImpl
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
EphemerisDataService::EphemerisDataService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new EphemerisDataServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addItemACK", &EphemerisDataService::addItemACK, this);
    registerSubscribe("deleteItemACK", &EphemerisDataService::deleteItemACK, this);
    registerSubscribe("modifyItemACK", &EphemerisDataService::modifyItemACK, this);
}

EphemerisDataService::~EphemerisDataService() { delete m_impl; }

void EphemerisDataService::addItem(const QByteArray& data)
{
    auto pack = EphemerisDataServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}
void EphemerisDataService::deleteItem(const QByteArray& data)
{
    auto pack = EphemerisDataServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}
void EphemerisDataService::modifyItem(const QByteArray& data)
{
    auto pack = EphemerisDataServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}

void EphemerisDataService::addItemACK(const ProtocolPack& pack) { emit sg_addACK(pack.data); }
void EphemerisDataService::deleteItemACK(const ProtocolPack& pack) { emit sg_deleteACK(pack.data); }
void EphemerisDataService::modifyItemACK(const ProtocolPack& pack) { emit sg_modifyACK(pack.data); }
