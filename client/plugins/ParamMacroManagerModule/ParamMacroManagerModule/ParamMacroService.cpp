#include "ParamMacroService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "ServerParamMacroService";

class ParamMacroServiceImpl
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

ParamMacroService::ParamMacroService(cppmicroservices::BundleContext context)
    : INetMsg()
    , mImpl(new ParamMacroServiceImpl)
{
    mImpl->context = context;

    registerSubscribe("addItemACK", &ParamMacroService::addItemACK, this);
    registerSubscribe("deleteItemACK", &ParamMacroService::deleteItemACK, this);
    registerSubscribe("modifyItemACK", &ParamMacroService::modifyItemACK, this);
}

ParamMacroService::~ParamMacroService() { delete mImpl; }

void ParamMacroService::addItemACK(const ProtocolPack& pack) { emit signalAddItemACK(pack.data); }

void ParamMacroService::deleteItemACK(const ProtocolPack& pack) { emit signalDeleteItemACK(pack.data); }

void ParamMacroService::modifyItemACK(const ProtocolPack& pack) { emit signalModifyItemACK(pack.data); }

void ParamMacroService::slotAddItem(const QByteArray& data)
{
    auto pack = ParamMacroServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}

void ParamMacroService::slotDeleteItem(const QByteArray& data)
{
    auto pack = ParamMacroServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}

void ParamMacroService::slotModifyItem(const QByteArray& data)
{
    auto pack = ParamMacroServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}
