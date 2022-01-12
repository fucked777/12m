#ifndef AGCDEFINE_H
#define AGCDEFINE_H

#include "ProtocolXmlTypeDefine.h"
#include "SystemWorkMode.h"
#include <QByteArray>
#include <QDataStream>
#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QVariant>

struct AGCResult
{
    // double ycAGC{ 0.0 }; /* 遥测agc电压 */
    // double gzAGC{ 0.0 }; /* 跟踪agc电压 */
    double agc{ 0.0 }; /* agc电压 */
    double sbf{ 0.0 }; /* sbf */
};
using AGCResultList = QList<AGCResult>;
struct AGCCalibrationItem
{
    QString itemGUID;
    QString taskCode;                                     /* 任务代号 6B */
    QDateTime createTime;                                 /* 创建时间 */
    QString projectCode;                                  /* 项目代号 6B */
    QString taskID;                                       /* 卫星标识\卫星标识 */
    SystemWorkMode workMode{ SystemWorkMode::NotDefine }; /* 工作模式 */
    quint32 dotDrequencyNum{ 0 };                         /* 点频 */
    quint64 equipComb{ 0 };                               /* 设备组合号 */
    AGCResultList result;

    // double downFreq{ 0 };                                 /* 下行频率[MHz] */
};
using AGCCalibrationItemList = QList<AGCCalibrationItem>;

struct AGCResultQuery
{
    QDateTime beginTime;
    QDateTime endTime;
    QString taskCode; /* 任务代号 */
};

struct ClearAGCCalibrationHistry
{
    quint64 timeInterval{ 30 * 3 }; /* 默认是90天 */
};

/* 标校的状态 */
enum class AGCCalibrationStatus
{
    End = 0x00, /* agc曲线标定结束 */
    Failed,     /* 失败 */
    Busy,       /* 忙(正在执行) */
    Info,       /* 过程状态信息 */
};
/* 标校过程应答 */
struct AGCCalibrationProcessACK
{
    AGCCalibrationStatus result{ AGCCalibrationStatus::Failed };
    QString msg;
    AGCResult value;
    AGCCalibrationItem item;
};

// struct AGCSendToBS
//{
//    DeviceID deviceID;
//    AGCCalibrationItem item;
//};

#endif  // AGCDEFINE_H
