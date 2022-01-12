#ifndef SQLTASKEXECUTEMANAGER_H
#define SQLTASKEXECUTEMANAGER_H

#include "Utility.h"

struct DeviceWorkTaskConditionQueryACK;
struct DeviceWorkTaskConditionQuery;
struct AutoTaskLogData;
class SqlTaskExecuteManager
{
public:
    SqlTaskExecuteManager();
    /* 根据条件查询历史设备工作计划 暂未使用 */
    static Optional<DeviceWorkTaskConditionQueryACK> getHistoryDevPlan(const DeviceWorkTaskConditionQuery& item);
    static Optional<QList<AutoTaskLogData>> queryAutoTaskLog(const QString& uuid, quint64 startTime, quint64 endTime);
};

#endif  // SQLTASKEXECUTEMANAGER_H
