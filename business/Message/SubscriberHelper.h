#ifndef SUBSCRIBERHELPER_H
#define SUBSCRIBERHELPER_H

#include "RedisHelper.h"
#include "HeartbeatMessage.h"
#include <QObject>

struct ControlCmdResponse;

struct SystemLogData;
struct AutoTaskLogData;
struct CurrentRunningTaskPlanData;
struct DeviceStatusData;

/* 此类如果有成员写入需要加锁 */
/* 此类如果有成员写入需要加锁 */
/* 此类如果有成员写入需要加锁 */
// 统一订阅消息，收到指定消息后触发信号
class SubscriberHelper : public QObject
{
    Q_OBJECT
public:
    SubscriberHelper();
    ~SubscriberHelper();

    static SubscriberHelper* getInstance();

protected:
    void timerEvent(QTimerEvent* event) override;

signals:
    // 控制命令响应可读
    void signalControlCmdResponseReadable(const ControlCmdResponse& cmdResponse);

    // 系统日志可读
    void signalSystemLogReadable(const SystemLogData& data);
    // 自动化运行任务日志可读
    void signalAutoRunTaskLogReadable(const AutoTaskLogData& data);
    // 自动运行任务步骤状态数据可读
    void signalAutoRunTaskStepStatusReadable(const CurrentRunningTaskPlanData& data);
    // 设备状态数据可读
    void signalDeviceStatusDataReadable(const DeviceStatusData& data);

    void sg_masterSlave(const HeartbeatMessage&msg);
    /* 主备机状态改变 */
    void sg_statusChange(const HeartbeatMessage&msg);

private:
    QString m_masterIP;
    bool m_isMaster{true};
    cpp_redis::subscriber mSuber;
    int m_timerID { -1 };
};

#endif  // SUBSCRIBERHELPER_H
