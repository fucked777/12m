#include "DMService.h"
#include "Utility.h"
#include "CppMicroServicesUtility.h"
#include <QMessageBox>

static constexpr auto serverSelfModule = "DeviceManagement";
class DMServiceImpl
{
    DIS_COPY_MOVE(DMServiceImpl)
public:
    inline DMServiceImpl() {}
    inline ~DMServiceImpl() {}
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
DMService::DMService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new DMServiceImpl)
{
    m_impl->context = context;

    registerSubscribe("addItemACK", &DMService::addItemACK, this);
    registerSubscribe("deleteItemACK", &DMService::deleteItemACK, this);
    // registerSubscribe("queryItemACK", &DMService::queryItemACK, this);
    registerSubscribe("modifyItemACK", &DMService::modifyItemACK, this);
}

DMService::~DMService() { delete m_impl; }

void DMService::addItem(const QByteArray& data)
{
    auto pack = DMServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}
void DMService::deleteItem(const QByteArray& data)
{
    auto pack = DMServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}
// void DMService::queryItem(const QByteArray& data)
//{
//    auto pack = DMServiceImpl::packServerSelf("queryItem", "queryItemACK", data);
//    silenceSend(pack);
//}
void DMService::modifyItem(const QByteArray& data)
{
    auto pack = DMServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}

void DMService::addItemACK(const ProtocolPack& pack) { emit sg_addACK(pack.data); }
void DMService::deleteItemACK(const ProtocolPack& pack) { emit sg_deleteACK(pack.data); }
// void DMService::queryItemACK(const ProtocolPack& pack) { emit sg_queryACK(pack.data); }
void DMService::modifyItemACK(const ProtocolPack& pack) { emit sg_modifyACK(pack.data); }
