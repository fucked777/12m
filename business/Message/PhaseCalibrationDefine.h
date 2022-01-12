#ifndef PHASECALIBRATIONDEFINE_H
#define PHASECALIBRATIONDEFINE_H

#include "SystemWorkMode.h"
#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>

/*
 * ACU校相频段
 */
enum class ACUPhaseCalibrationFreqBand
{
    Unknown = 0,    // 未知
    S = 1,          // S
    KaYC = 2,       // Ka遥测
    KaSC = 3,       // Ka数传
    SKaYC = 4,      // S+Ka遥测
    SKaSC = 5,      // S+Ka数传
    SKaYCKaSC = 6,  // S+Ka遥测+Ka数传
};
/* 校相极化方式 */
enum class PhaseCalibrationEmissPolar
{
    Left = 1,  /* 左旋 */
    Right = 2, /* 右旋 */
};
/* 校相方式 */
enum class PhaseCalibrationCorrMeth
{
    Tower = 1,     /* 对塔 */
    Current = 2,   /* 当前位置 */
    Satellite = 3, /* 对星(动态) */
};
/* 校相自跟踪检查 */
enum class PhaseCalibrationFollowCheck
{
    Not = 0,      /* 无(对塔有效) */
    Check = 1,    /* 检查 */
    NotCheck = 2, /* 不检查 */
};

enum class PhaseCalibrationStatus
{
    Error = 0,                              //未知错误
    Start = 1,                              // 开始校相
    Finish = 2,                             // 校相正常完成
    End = 3,                                // 结束校相
    NotFoundTrack = 4,                      // 未找到轨道数据
    BasebandQueryTimeout = 5,               // 基带查询超时
    BasebandPhaseCalibrationTimeout = 6,    // 基带校相超时
    BasebandBindingTimeout = 7,             // 基带装订超时
    BasebandRejectionPhaseCalibration = 8,  // 基带拒收校相
    BasebandRejectionBinding = 9,           // 基带拒收装订
    ReceiverOutOfLock = 10,                 // 接收机失锁
    StartSelfTrackingCheck = 11,            // 开始自跟踪检查
    FinishSelfTrackingCheck = 12,           // 完成自跟踪检查
    StartOSAndCCCheck = 13,                 // 开始定向灵敏度和交叉耦合检查
    FinishOSAndCCCheck = 14,                // 完成定向灵敏度和交叉耦合检查
    AGCAbnormal = 15,                       // AGC异常
    ErrorVoltageOverrun = 16,               // 误差电压超限
    BasebandBindingFailed = 17,             // 基带装订失败
};

enum class SelfTrackCheckResult
{
    NormalPolarity = 1,             /* 极性正常 */
    AZPolarityReversal = 2,         /* 方位极性反 */
    ELPolarityReversal = 3,         /* 俯仰极性反 */
    AZELPolarityReversal = 4,       /* 方位俯仰极性均反 */
    OutOfLock = 5,                  /* 检查时失锁 */
    AGCAbnormal = 6,                /* AGC异常 */
    AZRandomDifferenceAbnormal = 7, /* 方位随机差异常 */
    ElRandomDifferenceAbnormal = 8, /* 俯仰随机差异常 */
    NoCheck = 9,                    /* 未做检查 */
};

/* 校相需要的参数 */
struct PhaseCalibrationParameter
{
    // quint64 equipComb{ 0 };                                                        /* 设备组合号 校相参数用的不用任意给一个 */
    // PhaseCalibrationEmissPolar sEmissPolar{ PhaseCalibrationEmissPolar::Left };    /* S极化 */
    // PhaseCalibrationEmissPolar kaSCEmissPolar{ PhaseCalibrationEmissPolar::Left }; /* Ka数传极化 */
    // PhaseCalibrationEmissPolar kaYCEmissPolar{ PhaseCalibrationEmissPolar::Left }; /* Ka遥测极化 */

    QString projectCode;                                                           /* 项目代号 */
    QString taskIdent;                                                             /* 任务标识 */
    QString taskCode;                                                              /* 任务代号 */
    ACUPhaseCalibrationFreqBand freqBand{ ACUPhaseCalibrationFreqBand::S };        /* 校相频段 */
    PhaseCalibrationCorrMeth corrMeth{ PhaseCalibrationCorrMeth::Current };        /* 校相方式 */
    PhaseCalibrationFollowCheck followCheck{ PhaseCalibrationFollowCheck::Check }; /* 自跟踪检查 */
    quint32 crossLowLimit{ 0 };                                                    /* 交叉耦合下限值(分母) */
    qint32 standSensit{ 0 };                                                       /* 定向灵敏度标准值 */
    quint32 directToler{ 0 };                                                      /* 定向灵敏度允差 */
};
struct PhaseCalibrationSaveParameter
{
    ACUPhaseCalibrationFreqBand freqBand{ ACUPhaseCalibrationFreqBand::S }; /* 校相频段 */
    SystemWorkMode systemWorkMode{ SystemWorkMode::NotDefine };             /* 工作模式区别于校相的模式 */
    int dotDrequency{ -1 };                                                 /* 点频 */
    double downFreq{ 0 };                                                   /* 下行频率 */
};

/* 开始校相的参数 */
struct StartPhaseCalibration
{
    PhaseCalibrationParameter parameter; /* 校相指令的参数 */

    /* 保存时需要用到的的参数key是校相的模式
     * 校相时给ACU的模式如果是多个 比如 S+Ka遥测
     * 校相时会分成单体上报  就是S一个结果  Ka遥测一个结果
     * 所以这里的key就只是
     * S Ka遥测 Ka数传
     */
    QMap<ACUPhaseCalibrationFreqBand, PhaseCalibrationSaveParameter> saveParameter;
};

struct PhaseCalibrationItem
{
    QString id;                                           /* 唯一ID 任务代号_工作模式_下行频率_uuid*/
    QString projectCode;                                  /* 项目代号 */
    QString taskIdent;                                    /* 任务标识 */
    QString taskCode;                                     /* 任务代号 */
    QDateTime createTime;                                 /* 创建时间 */
    SystemWorkMode workMode{ SystemWorkMode::NotDefine }; /* 工作模式 */
    quint64 equipComb{ 0 };                               /* 设备组合号 */
    int dotDrequency{ -1 };                               /* 点频 */
    double downFreq{ 0 };                                 /* 下行频率 */
    double azpr{ 0.0 };                                   /* 方位移相值 */
    double elpr{ 0.0 };                                   /* 俯仰移相值 */
    double azge{ 0.0 };                                   /* 方位斜率 */
    double elge{ 0.0 };                                   /* 俯仰斜率 */
    double temperature{ 0.0 };                            /* 中心体温度 */
    double humidity{ 0.0 };                               /* 中心体湿度 */
    /* 这个是原始的结果数据
     * 字段只有部分有用的数据
     */
    StartPhaseCalibration rawParameter; /* 校相的原始参数 */
};

/* 清理历史记录 */
struct ClearPhaseCalibrationHistry
{
    quint32 timeInterval{ 30 * 3 }; /* 默认是90天 */
};
struct QueryPhaseCalibrationHistry
{
    SystemWorkMode workMode{ SystemWorkMode::NotDefine }; /* 工作模式 */
    QString taskCode;                                     // 任务代号
    QDateTime beginTime;
    QDateTime endTime;
};

using PhaseCalibrationItemList = QList<PhaseCalibrationItem>;
Q_DECLARE_METATYPE(PhaseCalibrationItem);
Q_DECLARE_METATYPE(StartPhaseCalibration);
Q_DECLARE_METATYPE(PhaseCalibrationParameter);
Q_DECLARE_METATYPE(PhaseCalibrationSaveParameter);
Q_DECLARE_METATYPE(ClearPhaseCalibrationHistry);
Q_DECLARE_METATYPE(QueryPhaseCalibrationHistry);
#endif  // PHASECALIBRATIONDEFINE_H
