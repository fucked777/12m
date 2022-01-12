#ifndef HEARTBEATMESSAGE_H
#define HEARTBEATMESSAGE_H
#include <QString>
#include <QObject>
#include "TimeMessageDefine.h"

enum class MasterSlaveStatus
{
    Unknown,
    Master,
    Slave,
};

struct HeartbeatMessage
{
    bool isUsed{false};                                         /* 配置主备是否启用 */
    MasterSlaveStatus status{MasterSlaveStatus::Unknown};       /* 是否是主机 只有服务器有用 */
    QString serverID;                                           /* 主用服务的服务ID */
    QString currentMasterIP;                                    /* 当前主机的IP */
};

Q_DECLARE_METATYPE(HeartbeatMessage);

#endif  // HEARTBEATMESSAGE_H
