#ifndef AUTORUNTASK4426SERVICE_H
#define AUTORUNTASK4426SERVICE_H

#include "INetMsg.h"
#include "HeartbeatMessage.h"

namespace cppmicroservices
{
    class BundleContext;
}

struct ManualMessage;
struct AutoTaskLogData;
class TaskScheduling;
class StationNetCenterCmd;
class AutoRunTask4426Service : public INetMsg
{
public:
    AutoRunTask4426Service(cppmicroservices::BundleContext context);
    ~AutoRunTask4426Service();

private:
    void sendByteArrayToDevice(const QByteArray& data);
    /* 接收战网中心的数据 */
    void recvStationNetCenter(const ProtocolPack& pack);

    /* 接收中心来的设备工作计划*/
    void recvDeviceWorkTaskList(const ProtocolPack& pack);

    /* 任务资源释放 */
    void taskResourceRelease(const ManualMessage& manualMessage);
private slots:
    // 读取自动运行任务日志
    void slotReadAutoRunTaskLog(const AutoTaskLogData& data);
    /* 主备切换 */
    void masterSlave(const HeartbeatMessage& msg);

private:
    TaskScheduling* mTaskScheduling = nullptr;
    StationNetCenterCmd* mStationNetCenter = nullptr;
    MasterSlaveStatus mMasterStatus{MasterSlaveStatus::Unknown};
};

#endif  // AUTORUNTASK4426SERVICE_H
