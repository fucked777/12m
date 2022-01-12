#include "TaskExecuteLogService.h"
#include "AutoRunTaskLogMessageSerialize.h"
#include "CppMicroServicesUtility.h"
#include "SqlTaskExecuteManager.h"
#include "TaskPlanMessageSerialize.h"

class TaskExecuteLogServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlTaskExecuteManager sql;
};

TaskExecuteLogService::TaskExecuteLogService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new TaskExecuteLogServiceImpl)
{
    m_impl->context = context;
}

TaskExecuteLogService::~TaskExecuteLogService() { delete m_impl; }

void TaskExecuteLogService::slotGetHistoryPlan(const QByteArray& data)
{
    QByteArray tempData = data;
    DeviceWorkTaskConditionQuery item;
    tempData >> item;
    tempData.clear();
    tempData << m_impl->sql.getHistoryDevPlan(item);
    emit signalGetHistoryPlanACK(tempData);
}

void TaskExecuteLogService::slotQueryAutoTaskLog(const QByteArray& data)
{
    QByteArray tempData = data;
    AutoTaskLogDataCondition item;
    tempData >> item;
    tempData.clear();
    tempData << m_impl->sql.queryAutoTaskLog(item.uuid, item.startTime, item.endTime);
    emit signalQueryAutoTaskLogACK(tempData);
}
