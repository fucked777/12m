#ifndef TASKPLANMANAGERSERVICE_H
#define TASKPLANMANAGERSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}
struct DataTranWorkTask;
using DataTranWorkTaskList = QList<DataTranWorkTask>;
class TaskPlanManagerServiceImpl;
class TaskPlanManagerService : public INetMsg
{
public:
    TaskPlanManagerService(cppmicroservices::BundleContext context);
    ~TaskPlanManagerService();

private:
    // 添加数传计划
    void addItem(const ProtocolPack&);
    // 删除计划
    void deletePlanItem(const ProtocolPack&);
    // 查看计划
    void viewItem(const ProtocolPack&);
    // 获取编辑计划的信息
    void editInfoItem(const ProtocolPack&);
    // 更新计划的信息
    void updateItem(const ProtocolPack&);
    // 查询计划
    void queryDeviceItem(const ProtocolPack&);
    // 查询计划
    void queryDTItem(const ProtocolPack&);
    // 更新计划状态，这里会同步更新数传的状态
    void modifyPlanStatus(const ProtocolPack& pack);
    // 根据结构体 DeviceWorkTaskConditionQuery 做查询,结构体可以参考 TaskPlanMessageDefine.h
    void getHistoryPlan(const ProtocolPack& pack);
    // 根据任务代号查询自动化运行日志
    void queryAutoTaskLog(const ProtocolPack& pack);

    void initParseTaskPlanXml();

    //void newDataTransWorkTask(const DataTranWorkTaskList& dataTranWorkTaskList);

private:
    TaskPlanManagerServiceImpl* mImpl;
};
#endif  // TASKPLANMANAGERSERVICE_H
