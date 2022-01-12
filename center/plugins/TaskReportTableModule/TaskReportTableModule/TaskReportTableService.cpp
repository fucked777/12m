#include "TaskReportTableService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "ServerTaskReportTableService";

class TaskReportTableServiceImpl
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

    static inline ProtocolPack packServerSelf(const char* moduleName, const char* operation, const char* operationACK = "",
                                              const QByteArray& data = QByteArray())
    {
        ProtocolPack sendPack;
        sendPack.desID = moduleName;
        sendPack.operation = operation;
        sendPack.operationACK = operationACK;
        sendPack.module = false;
        sendPack.data = data;
        return sendPack;
    }
};
TaskReportTableService::TaskReportTableService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new TaskReportTableServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("GetHistoryPlanACK", &TaskReportTableService::getHistoryPlanACK, this);
    registerSubscribe("getDataByUUIDACK", &TaskReportTableService::getDataByUUIDACK, this);
}

TaskReportTableService::~TaskReportTableService() { delete m_impl; }

void TaskReportTableService::getHistoryPlanACK(const ProtocolPack& pack) { emit signalGetHistoryPlanACK(pack.data); }

void TaskReportTableService::getDataByUUIDACK(const ProtocolPack& pack) { emit signalGetDataByUUIDACK(pack.data); }

void TaskReportTableService::slotGetHistoryPlan(const QByteArray& data)
{
    auto pack = TaskReportTableServiceImpl::packServerSelf("TaskPlanManagerService", "GetHistoryPlan", "GetHistoryPlanACK", data);
    silenceSend(pack);
}

void TaskReportTableService::slotGetDataByUUID(const QByteArray& data)
{
    auto pack = TaskReportTableServiceImpl::packServerSelf("getDataByUUID", "getDataByUUIDACK", data);
    silenceSend(pack);
}
