#ifndef DEVICESTATUSLOGMESSAGEDEFINE_H
#define DEVICESTATUSLOGMESSAGEDEFINE_H

#include <QString>

#include "GlobalDefine.h"

struct DeviceStatusLogDataDev
{
    quint64 id;          // 日志唯一ID
    QString createTime;  // 日志时间
    QString deviceId;
    QString unitId;
    QString param;
    QString paramValue;
};

#endif  // DEVICESTATUSLOGMESSAGEDEFINE_H
