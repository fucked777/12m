#include "ResourceRestructuringService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "ServerResourceRestructuringService";

class ResourceRestructuringServiceImpl
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
ResourceRestructuringService::ResourceRestructuringService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new ResourceRestructuringServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addItemACK", &ResourceRestructuringService::addItemACK, this);
    registerSubscribe("deleteItemACK", &ResourceRestructuringService::deleteItemACK, this);
    registerSubscribe("queryItemACK", &ResourceRestructuringService::queryItemACK, this);
    registerSubscribe("modifyItemACK", &ResourceRestructuringService::modifyItemACK, this);
}

ResourceRestructuringService::~ResourceRestructuringService() { delete m_impl; }

void ResourceRestructuringService::addItemACK(const ProtocolPack& pack) { emit signalAddItemACK(pack.data); }

void ResourceRestructuringService::deleteItemACK(const ProtocolPack& pack)
{
    emit signalDeleteItemACK(pack.data);
}

void ResourceRestructuringService::queryItemACK(const ProtocolPack &pack){ emit signalQueryItemACK(pack.data); }

void ResourceRestructuringService::modifyItemACK(const ProtocolPack& pack) { emit signalModifyItemACK(pack.data); }

void ResourceRestructuringService::slotAddItem(const QByteArray& data)
{
    auto pack = ResourceRestructuringServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}

void ResourceRestructuringService::slotDeleteItem(const QByteArray& data)
{
    auto pack = ResourceRestructuringServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}

void ResourceRestructuringService::slotQueryItem(const QByteArray &data)
{
    auto pack = ResourceRestructuringServiceImpl::packServerSelf("queryItem", "queryItemACK", data);
    silenceSend(pack);
}

void ResourceRestructuringService::slotModifyItem(const QByteArray& data)
{
    auto pack = ResourceRestructuringServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}
