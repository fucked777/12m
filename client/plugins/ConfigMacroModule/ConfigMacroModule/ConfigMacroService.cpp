#include "ConfigMacroService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "ServerConfigMacroService";

class ConfigMacroServiceImpl
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

ConfigMacroService::ConfigMacroService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new ConfigMacroServiceImpl)
{
    mImpl->context = context;
    registerSubscribe("addItemACK", &ConfigMacroService::addItemACK, this);
    registerSubscribe("deleteItemACK", &ConfigMacroService::deleteItemACK, this);
    registerSubscribe("modifyItemACK", &ConfigMacroService::modifyItemACK, this);
}

ConfigMacroService::~ConfigMacroService() { delete mImpl; }

void ConfigMacroService::addItemACK(const ProtocolPack& pack) { emit signalAddItemACK(pack.data); }

void ConfigMacroService::deleteItemACK(const ProtocolPack& pack) { emit signalDeleteItemACK(pack.data); }

void ConfigMacroService::modifyItemACK(const ProtocolPack& pack) { emit signalModifyItemACK(pack.data); }

void ConfigMacroService::slotAddItem(const QByteArray& data)
{
    auto pack = ConfigMacroServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}

void ConfigMacroService::slotDeleteItem(const QByteArray& data)
{
    auto pack = ConfigMacroServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}

void ConfigMacroService::slotModifyItem(const QByteArray& data)
{
    auto pack = ConfigMacroServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}
