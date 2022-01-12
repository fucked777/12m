#ifndef POWERCONTROLTYPE_H
#define POWERCONTROLTYPE_H

#include <QList>
#include <QString>

/* 排插的接口信息 */
struct PlugInfo
{
    QString host;           /* IP地址 */
    quint16 port{ 80 };     /* 端口 */
    quint8 no{ 0 };         /* 排插的接口编号 */
    QString name;           /* 接口的名称 */
    bool isEnable{ false }; /* 是否使用 */
};
struct PlugDeviceInfo
{
    QString name;       /* 设备的名称 */
    QString host;       /* IP地址 */
    quint16 port{ 80 }; /* 端口 */
};

/* 单个机柜的信息 */
struct PowerStripInfo
{
    QString name;                     /* 机柜名称 */
    QList<PlugInfo> plugList;         /* 排插接口的信息 */
    QList<PlugDeviceInfo> deviceList; /* 排插设备的信息 */
};

/* 中心体设备的信息 */
struct CenterBodyInfo
{
    QString type;       /* 只有两个值 unit 和 process */
    QString name;       /*设备名*/
    quint64 devID{ 0 }; /* 设备ID */
    int cmdID{ -1 };    /* unit代表 单元ID process代表命令ID */
    QString key;        /* 指令的key */
    int on{ -1 };       /* 开机的value */
    int off{ -1 };      /* 关机的value */
};

struct SHPPowerInfo
{
    QString ip;
    int recvPort{ 0 };
    int sendPort{ 0 };
};

#endif  // POWERCONTROLTYPE_H
