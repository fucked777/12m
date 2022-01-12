#ifndef BLOCKDIAGRAMDEFINE_H
#define BLOCKDIAGRAMDEFINE_H
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"
#include <QList>
#include <QMap>
#include <QObject>

/* 类型 根据此类型 执行的操作不同 */
enum class ItemShowType
{
    Status = 0x01,          //设备状态      显示状态 点击时会弹出当前设备的信息
    TwoValueSwitch = 0x02,  //二值开关    显示状态 点击时切换开关状态
    System = 0x03,          //分系统       不显示状态 点击时会弹出当前的分系统
};

enum class DevStatusColorType
{
    None,
    GGR,  //灰绿红
    GRG,  //灰红绿
    GR,   // 1绿 2灰
};
struct UnitIDKeyInfo
{
    qint32 unitID;
    QString key;
    UnitIDKeyInfo()
        : unitID(0)
    {
    }
    UnitIDKeyInfo(int v1, const QString& v2)
        : unitID(v1)
        , key(v2)
    {
    }
};
struct UnitIDKeyValueInfo
{
    qint32 unitID;
    QString key;
    qint32 triggerValue;
    UnitIDKeyValueInfo()
        : unitID(0)
        , triggerValue(0)
    {
    }
    UnitIDKeyValueInfo(int v1, const QString& v2, const qint32& v3)
        : unitID(v1)
        , key(v2)
        , triggerValue(v3)
    {
    }
};

using StatusID = UnitIDKeyInfo;
using ControlID = UnitIDKeyInfo;
using ShutdownID = UnitIDKeyValueInfo;

/* 分系统
 * 不需要显示状态,只需要点击时弹出当前分系统窗体就行
 */
struct SystemTypeInfo
{
    QString graphicsID;          /* 图元ID */
    QString statusTriggerModule; /* 双击图元触发的模块名 */
    QString systemID;            /* 分系统ID,双击后根据此名字找窗体 */
    QString systemName;          /* 分系统名,显示标题的 */
};

/*
 *补充命令信息，考虑了2种情况：
 * 1.单元参数命令,相同单元的参数必须全部放在map里面
 * 2.过程控制命令
 * 目前不支持动态取值，以后用到了再添加
 */
struct PlusCmdInfo
{
    QMap<int, QMap<QString, QVariant>> unitParam;      /*key为unitId, QMap<QString, QVariant>为单元参数需要控制的值*/
    QMap<int, QMap<QString, QVariant>> processControl; /*key为cmdId, QMap<QString, QVariant>为过程控制命令需要控制的值*/
};

/*二值开关补充信息*/
struct TwoValueSwitchAddtionalInfo
{
    QMap<int, PlusCmdInfo> addtionalInfo; /*key为二值开关的值, QMap<QString, QVariant>为单元参数需要控制的值*/
};

/* 二值开关的信息 */
struct TwoValueSwitchTypeInfo
{
    QString graphicsID;                                     /* 图元ID */
    QString statusKey;                                      /* 状态的Key */
    QString cmdKey;                                         /* 命令的Key */
    std::tuple<int, double, QString> twoValueRelational[2]; /* 二值映射 第一个是开关本来的数据  第二个是图元的角度 第三个是提示 */
    int systemNum{ 0 };                                     /* 系统号 */
    int deviceNum{ 0 };                                     /* 设备号 */
    int extenNum{ 0 };                                      /* 分机号 */
    int modeNum{ 0 };                                       /* 当前设备模式号 */
    int cmdNum{ 0 };                                        /* 当前的命令号 */

    qint32 unitID{ -1 }; /* 单元号 */
    QString redisKey;    /* 从redis中查找上报数据的key */

    TwoValueSwitchAddtionalInfo addtionalInfo;
};

/* 主备信息 */
struct MainPreparationInfo
{
    QString mainRedisKey; /* 主机的redisKey 这里是考虑有些设备判断主备不是在同一台设备上 */
    QString redisKey;     /* 从redis中查找上报数据的key */
    QString indexKey;     /* 主备的Key */
    // quint32 deviceID{ 0 }; /* 设备ID */
    qint32 unitID{ -1 }; /* 单元号 */
    qint32 mainValue;    /* 主机的值 */
};
/* 加去电信息 这里直接认为数据来源是主机的数据 */
struct ShutdownInfo
{
    qint32 unitID;        /* 单元ID */
    QString indexKey;     /* 加去电的key */
    double shutdownValue; /* 去电时的值 */
};
/* 本分控 这里直接认为数据来源是主机的数据 */
struct ControlInfo
{
    qint32 unitID; /* 单元ID */
    // qint32 control{ 0 };    /* 本控 */
    qint32 subControl{ 0 }; /* 分控 */
    QString indexKey;       /* 加去电的key */
};
/* 综合状态 这里直接认为数据来源是主机的数据 */
struct ComprehensiveStateInfo
{
    qint32 unitID;      /* 单元ID */
    qint32 normal{ 0 }; /* 综合状态正常,除了正常全部判定为异常 */
    QString indexKey;   /* 加去电的key */
};

/* 设备状态信息显示 */
struct DeviceStatusTypeInfo
{
    QString graphicsID;          /* 图元ID */
    QString statusTriggerModule; /* 双击图元触发的模块名 */
    QString systemID;            /* 分系统ID,双击后根据此名字找窗体 */
    QString systemName;          /* 分系统名,显示标题的 */
    QString redisKey;            /* 从redis中查找上报数据的key */

    QList<MainPreparationInfo> mainPreparationList; /* 主备的信息 */
    ShutdownInfo shutdownInfo;                      /* 加去电信息 */
    ComprehensiveStateInfo comprehensiveStateInfo;  /* 综合状态 */
    ControlInfo controlInfo;                        /* 本分控信息 */
};

struct SystemBlockInfo
{
    /* 图元ID与图元数据的映射表 */
    QMap<QString, std::tuple<ItemShowType, QVariant>> dataConfigMap;
    QMap<QString, TwoValueSwitchTypeInfo> switchTypeMap;     /* 开关的数据 */
    QMap<QString, DeviceStatusTypeInfo> deviceStatusTypeMap; /* 设备的状态数据 */
    QMap<QString, QString> ribbonIcons;                      /* 系统框图各个分系统的对应的小图标 */
    qreal scaleX{ 1 };
    qreal scaleY{ 1 };

    /* 默认的颜色显示 */
    quint32 rgbOffLine{ 0 };
    quint32 rgbSubControl{ 0 };
    quint32 rgbFault{ 0 };
    quint32 rgbNormal{ 0 };
    quint32 rgbCurLink{ 0 };
    quint32 rgbNotCurLink{ 0 };
    quint32 rgbShutdown{ 0 };
    quint32 rgbSystem{ 0 };
    quint32 rgbNotFound{ 0 };
    quint32 rgbDefault{ 0 };
};

Q_DECLARE_METATYPE(TwoValueSwitchTypeInfo)
Q_DECLARE_METATYPE(SystemTypeInfo)
Q_DECLARE_METATYPE(DeviceStatusTypeInfo)

#endif  // BLOCKDIAGRAMDEFINE_H

// struct DeviceStatusTypeInfo
//{
//    ItemShowType showType{ ItemShowType::Status }; /* 显示类型 */
//    QString systemID;                              /* 分系统ID,双击后根据此名字找窗体 */
//    QString systemName;                            /* 分系统名,显示标题的 */

//    /* 开关的数据映射表 开关状态 对应多少度 */
//    QList<std::tuple<int, double>> dataRelational;

//    DeviceID deviceID; /* 当前的设备ID */
//    QString redisKey;  /* 从redis中查找上报数据的key */
//    int modeID{ -1 };
//    StatusID statusID;     /* 状态 */
//    ControlID controlID;   /* 开关 */
//    ShutdownID shutdownID; /* 开关电 */
//    QString graphicsID;    //系统框图中，对应设备的框图ID
//    QString desc;
//    /* 20210317 主备信息 */
//    MainPreparationInfo mainPreparationInfo;
//    /* 20200805只有开关切换用得上 */
//    /* 提示信息 */
//    QStringList tips;
//    /* 命令ID */
//    int cmdID{ -1 };
//};
