#include "TaskCenterService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "TaskCenter";
class TaskCenterServiceImpl
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
TaskCenterService::TaskCenterService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new TaskCenterServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addItemACK", &TaskCenterService::addItemACK, this);
    registerSubscribe("deleteItemACK", &TaskCenterService::deleteItemACK, this);
    registerSubscribe("modifyItemACK", &TaskCenterService::modifyItemACK, this);
}

TaskCenterService::~TaskCenterService() { delete m_impl; }

void TaskCenterService::addItem(const QByteArray& data)
{
    auto pack = TaskCenterServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}
void TaskCenterService::deleteItem(const QByteArray& data)
{
    auto pack = TaskCenterServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}
void TaskCenterService::modifyItem(const QByteArray& data)
{
    auto pack = TaskCenterServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}

void TaskCenterService::addItemACK(const ProtocolPack& pack) { emit sg_addACK(pack.data); }
void TaskCenterService::deleteItemACK(const ProtocolPack& pack) { emit sg_deleteACK(pack.data); }
void TaskCenterService::modifyItemACK(const ProtocolPack& pack) { emit sg_modifyACK(pack.data); }
