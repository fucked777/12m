#ifndef DISTANCECALIBRATIONDEFINE_H
#define DISTANCECALIBRATIONDEFINE_H

#include "SystemWorkMode.h"
#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>

enum class ChannelSel
{
    None,
    YTH = 0x01, /* 一体化通道 */
    KP = 0x02,  /* 扩频通道 */
};

struct DistanceCalibrationItem
{
    QString id;                                           /* 唯一ID */
    QString projectCode;                                  /* 项目代号 */
    QString taskIdent;                                    /* 任务标识 */
    QString taskCode;                                     /* 任务代号 */
    int dpNum;                                            /* 点频信息 add by wjy */
    QDateTime createTime;                                 /* 创建时间 */
    SystemWorkMode workMode{ SystemWorkMode::NotDefine }; /* 工作模式 */
    qint32 channel{ 0 };                                  /* 通道选择，sttc无效, 扩频是1-4 扩跳是1-2 */
    quint64 equipComb{ 0 };                               /* 设备组合号 */
    double uplinkFreq{ 0 };                               /* 上行频率 */
    double downFreq{ 0 };                                 /* 下行频率 */
    double correctValue{ 0.0 };                           /* 修正值 */
    quint32 statisPoint{ 0 };                             /* 统计点数 */
    quint32 signalRot{ 0 };                               /* 旋向 */

    /* 这个是结果 */
    double distZeroMean{ 0.0 }; /* 距离零值均值 */
    double distZeroVar{ 0.0 };  /* 距离零值方差 */
};

/* 清理历史记录 */
struct ClearDistanceCalibrationHistry
{
    quint32 timeInterval{ 30 * 3 }; /* 默认是90天 */
};
struct QueryDistanceCalibrationHistry
{
    QString taskCode;  // 任务代号
    QDateTime beginTime;
    QDateTime endTime;
};

using DistanceCalibrationItemList = QList<DistanceCalibrationItem>;
Q_DECLARE_METATYPE(DistanceCalibrationItem);
Q_DECLARE_METATYPE(ClearDistanceCalibrationHistry);
Q_DECLARE_METATYPE(QueryDistanceCalibrationHistry);
#endif  // DCDEFINE_H
