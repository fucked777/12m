#include "TaskPlanManagerService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "TaskPlanManagerService";

class TaskPlanManagerServiceImpl
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
TaskPlanManagerService::TaskPlanManagerService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new TaskPlanManagerServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addItemACK", &TaskPlanManagerService::addItemACK, this);
    registerSubscribe("deletePlanItemACK", &TaskPlanManagerService::deletePlanItemACK, this);
    registerSubscribe("viewItemACK", &TaskPlanManagerService::viewItemACK, this);
    registerSubscribe("editInfoItemACK", &TaskPlanManagerService::editInfoItemACK, this);
    registerSubscribe("updateItemACK", &TaskPlanManagerService::updateItemACK, this);
    // registerSubscribe("queryDeviceItemACK", &TaskPlanManagerService::queryDeviceItemACK, this);
    // registerSubscribe("queryDTItemACK", &TaskPlanManagerService::queryDTItemACK, this);
}

TaskPlanManagerService::~TaskPlanManagerService() { delete m_impl; }

void TaskPlanManagerService::addItemACK(const ProtocolPack& pack) { emit sg_addItemACK(pack.data); }
void TaskPlanManagerService::deletePlanItemACK(const ProtocolPack& pack) { emit sg_deletePlanItemACK(pack.data); }
void TaskPlanManagerService::viewItemACK(const ProtocolPack& pack) { emit sg_viewItemACK(pack.data); }
void TaskPlanManagerService::editInfoItemACK(const ProtocolPack& pack) { emit sg_editInfoItemACK(pack.data); }
void TaskPlanManagerService::updateItemACK(const ProtocolPack& pack) { emit sg_updateItemACK(pack.data); }
// void TaskPlanManagerService::queryDeviceItemACK(const ProtocolPack& pack) { emit sg_queryDeviceItemACK(pack.data); }
// void TaskPlanManagerService::queryDTItemACK(const ProtocolPack& pack) { emit sg_queryDTItemACK(pack.data); }

void TaskPlanManagerService::addItem(const QByteArray& data)
{
    auto pack = TaskPlanManagerServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}
void TaskPlanManagerService::deletePlanItem(const QByteArray& data)
{
    auto pack = TaskPlanManagerServiceImpl::packServerSelf("deletePlanItem", "deletePlanItemACK", data);
    silenceSend(pack);
}
void TaskPlanManagerService::viewItem(const QByteArray& data)
{
    auto pack = TaskPlanManagerServiceImpl::packServerSelf("viewItem", "viewItemACK", data);
    silenceSend(pack);
}
void TaskPlanManagerService::editInfoItem(const QByteArray& data)
{
    auto pack = TaskPlanManagerServiceImpl::packServerSelf("editInfoItem", "editInfoItemACK", data);
    silenceSend(pack);
}
void TaskPlanManagerService::updateItem(const QByteArray& data)
{
    auto pack = TaskPlanManagerServiceImpl::packServerSelf("updateItem", "updateItemACK", data);
    silenceSend(pack);
}

// void TaskPlanManagerService::queryDeviceItem(const QByteArray& data)
//{
//    auto pack = TaskPlanManagerServiceImpl::packServerSelf("queryDeviceItem", "queryDeviceItemACK", data);
//    silenceSend(pack);
//}
// void TaskPlanManagerService::queryDTItem(const QByteArray& data)
//{
//    auto pack = TaskPlanManagerServiceImpl::packServerSelf("queryDTItem", "queryDTItemACK", data);
//    silenceSend(pack);
//}
