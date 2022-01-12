#ifndef STATUSPERSISTENCEMESSAGEDEFINE_H
#define STATUSPERSISTENCEMESSAGEDEFINE_H

#include <QMap>
#include <QVariant>

#include "DevProtocol.h"
#include "GlobalDefine.h"
#include "ProtocolXmlTypeDefine.h"

struct ParamStatus
{
    QString dataType;       // 参数数据类型
    QString parName;        // 参数名称
    QString step;           // 步进
    QVariant value;         // 值
    QString unit;           // 单位
    QVariant minValue;      // 最小值
    QVariant maxValue;      // 最大值
    QString enumValueDesc;  // 枚举值描述 如果该参数是枚举类型，对应的值的描述
};

struct UnitStatusParam
{
    QMap<QString, ParamStatus> paramStatusMap;  // 状态参数 QMap<单元号和参数顺序, 参数信息>  单元号和参数顺序:4001 表示第4个单元第1个参数
};

struct DeviceStatusData
{
    DeviceStatusData() { qRegisterMetaType<DeviceStatusData>("DeviceStatusData"); }

    DeviceID deviceID;
    QString deviceName;
    int modeId{ 0xFFFF };
    QString modelName;
    QString proName{ "TK4424" };

    QMap<QString, UnitStatusParam> unitParamMap;  // 单元状态参数 QMap<单元名称, 单元参数>
};

struct DeviceStatusDataCondition
{
    QStringList devices;
    QStringList units;
    QStringList sids;
    QDateTime start_time;
    QDateTime end_time;
};

struct DeviceStatusLogData
{
    quint64 id{0};          // 日志唯一ID
    QDateTime createTime;  // 日志时间
    QString deviceId;
    QString unitId;
    QString param;
    QString paramValue;
    int modeId{0};
};

using UnitStatusParamMap = QMap<QString, UnitStatusParam>;
using ParamStatusMap = QMap<QString, ParamStatus>;
using DeviceStatusLogDataList = QList<DeviceStatusLogData>;

#endif  // STATUSPERSISTENCEMESSAGEDEFINE_H
