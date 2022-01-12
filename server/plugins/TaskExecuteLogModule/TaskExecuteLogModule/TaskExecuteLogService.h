#ifndef TaskExecuteLogSERVICE_H
#define TaskExecuteLogSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class TaskExecuteLogServiceImpl;
class TaskExecuteLogService : public INetMsg
{
public:
    TaskExecuteLogService(cppmicroservices::BundleContext context);
    ~TaskExecuteLogService();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void GetAllDeviceInfo(const ProtocolPack& pack);
    void GetCurrentPageDeviceInfo(const ProtocolPack& pack);
    void InsertDeviceWorkTask(const ProtocolPack& pack);
    void UpdataTaskStatus(const ProtocolPack& pack);
    void DeleteDeviceWorkTask(const ProtocolPack& pack);
    void GetTodayDeviceWorkTaskInfo(const ProtocolPack& pack);
    void GetHistoryPlan(const ProtocolPack& pack);
    void GetRunningOrNoStartDeviceInfo(const ProtocolPack& pack);

    void GetAllDataTranPlanInfoSS(const ProtocolPack& pack);
    void GetAllDataTranPlanInfoSH(const ProtocolPack& pack);
    void GetCurrentPageDataTranPlanInfoSS(const ProtocolPack& pack);
    void GetCurrentPageDataTranPlanInfoSH(const ProtocolPack& pack);
    void InsertDataTranPlan(const ProtocolPack& pack);
    void InsertPlanTask(const ProtocolPack& pack);
    void DeleteDataTranPlan(const ProtocolPack& pack);

private:
    TaskExecuteLogServiceImpl* mImpl;
};

#endif  // TaskExecuteLogSERVICE_H
