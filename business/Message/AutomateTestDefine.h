#ifndef AUTOMATETESTDEFINE_H
#define AUTOMATETESTDEFINE_H

#include "SystemWorkMode.h"
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>

/* 自动化测试的测试参数 或者结果参数的每一个项的数据 */
struct AutomateTestItemParameterInfo
{
    QString id;           /* 当前项的id */
    QVariant rawValue;    /* 当前项的原始值 */
    QString displayName;  /* 当前项的显示名称 */
    QString displayValue; /* 当前项的显示值 */
    AutomateTestItemParameterInfo() {}
    explicit AutomateTestItemParameterInfo(const QVariant& rawValue_)
        : rawValue(rawValue_)
    {
    }
};

using AutomateTestItemParameterInfoList = QList<AutomateTestItemParameterInfo>;
using AutomateTestItemParameterInfoMap = QMap<QString, AutomateTestItemParameterInfo>;
/* 自动化测试结果分组 */
struct AutomateTestResultGroup
{
    QString groupName;                            /* 当前结果的组名称 */
    AutomateTestItemParameterInfoList resultList; /* 当前分组的结果列表  */
};
using AutomateTestResultGroupList = QList<AutomateTestResultGroup>;
/* 测试结果查询 */
struct AutomateTestResultQuery
{
    QString testTypeID;  /* 测试的类型ID */
    QDateTime beginTime; /* 开始时间 */
    QDateTime endTime;   /* 结束时间 */
};

/* 参数装订 */
struct AutomateTestParameterBind
{
public:
    QString id;                                            /* 存储的ID */
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };  /* 参数装订的工作模式 */
    QString testTypeID;                                    /* 测试类型ID 决定进行那种测试 */
    QDateTime createTime;                                  /* 创建时间 */
    AutomateTestItemParameterInfoMap testBindParameterMap; /* 参数装订的参数Map */
};
/* 参数绑定分组 */
struct AutomateTestParameterBindGroup
{
public:
    QString testTypeID;                            /* 绑定的类型ID */
    QList<AutomateTestParameterBind> bindDataList; /* 当前绑定数据的列表 */
};
using AutomateTestParameterBindMap = QMap<QString, AutomateTestParameterBindGroup>;

/* 测试计划的项 */
struct AutomateTestPlanItem
{
    QString id;                                           /* 计划的唯一ID */
    QString testName;                                     /* 当前的测试名称 */
    QString testDesc;                                     /* 当前的测试描述 */
    QString groupName;                                    /* 测试组名称/测试分系统名称 */
    QString testTypeID;                                   /* 测试类型ID 决定进行那种测试 */
    QString testTypeName;                                 /* 对应测试的名称比如G/T值测试 本来是和testTypeID是一一对应的 */
    QString projectCode;                                  /* 项目代号 */
    QString taskIdent;                                    /* 任务标识 */
    QString taskCode;                                     /* 任务代号 */
    QDateTime createTime;                                 /* 创建时间 */
    SystemWorkMode workMode{ SystemWorkMode::NotDefine }; /* 工作模式 */
    quint64 equipComb{ 0 };                               /* 设备组合号 */
    qint32 dotDrequency{ 0 };                             /* 点频 */
    AutomateTestItemParameterInfoMap testParameterMap;    /* 测试参数Map */
    AutomateTestParameterBind automateTestParameterBind;  /* 参数装订的数据 */

    // 因为参数宏是可以更改上下行频率的
    // 假设因为频率填写错了改了一下 如果这里是保存数据的时候记录的数据
    // 一改这个计划就废了 无法使用了
    // double upFreq{ 0.0 };                                 /* 上行频率[MHz] */
    // double downFreq{ 0.0 };                               /* 下行频率[MHz] */

    // QList<AutomateTestResultGroup> testResultList;                 /* 测试结果参数 20210526 待定 */
};
using AutomateTestPlanItemList = QList<AutomateTestPlanItem>;

/* 测试计划的结果 */
struct AutomateTestResult
{
public:
    QString id;         /* 唯一ID */
    QString testTypeID; /* 测试类型ID 决定进行那种测试 */
    /* 对应测试的名称比如G/T值测试 本来是和testTypeID是一一对应的,但是当前测试项不存在了以前的结果可以接着看 */
    QString testTypeName;
    QString testPlanName;                       /* 对应创建测试项的名称 */
    QDateTime testTime;                         /* 测试时间 */
    AutomateTestPlanItem testParam;             /* 当前测试项的测试参数 */
    AutomateTestResultGroupList testResultList; /* 测试结果列表分组 */
};

using AutomateTestResultList = QList<AutomateTestResult>;
/* 清理历史记录 */
struct ClearAutomateTestHistry
{
public:
    quint64 timeInterval{ 30 * 3 }; /* 默认是90天 */
};

/* 自动化测试的状态 */
enum class AutomateTestStatus
{
    Failed = 0,        /* 失败 */
    Busy,              /* 忙(正在执行) */
    LinkConfiguration, /* 链路设置 */
    ParameterSetting,  /* 参数设置 */
    StartTesting,      /* 开始测试 */
    Testing,           /* 正在测试 */
    EndOfTest,         /* 测试结束 */
};
struct TestProcessACK
{
    AutomateTestStatus status{ AutomateTestStatus::Failed }; /* 当前的状态 */
    QString msg;                                             /* 错误信息 */
};

Q_DECLARE_METATYPE(AutomateTestItemParameterInfo);
Q_DECLARE_METATYPE(AutomateTestResultGroup);
Q_DECLARE_METATYPE(AutomateTestPlanItem);
Q_DECLARE_METATYPE(AutomateTestResult);
Q_DECLARE_METATYPE(AutomateTestResultQuery);
Q_DECLARE_METATYPE(AutomateTestParameterBind);
Q_DECLARE_METATYPE(AutomateTestParameterBindGroup);
Q_DECLARE_METATYPE(ClearAutomateTestHistry);
Q_DECLARE_METATYPE(TestProcessACK);

#endif  // AUTOMATETESTDEFINE_H
