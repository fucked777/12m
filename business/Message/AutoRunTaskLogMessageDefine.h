#ifndef AUTORUNTASKLOGMESSAGEDEFINE_H
#define AUTORUNTASKLOGMESSAGEDEFINE_H

#include <QColor>
#include <QDateTime>
#include <QString>

#include "GlobalData.h"
#include "GlobalDefine.h"

/* 任务自动化运行日志 */

// 自动任务日志级别
enum class AutoTaskLogLevel
{
    Info = 0,       // 提示
    Warning,        // 警告
    Error,          // 错误
    SpecificTips,   //
};

// 任务自动化运行日志
struct AutoTaskLogData
{
    QString uuid;                                      // 任务的唯一ID
    QString taskCode;                                  // 任务代号
    QString serialNumber;                              // 计划流水号
    QString createTime;                                // 日志生成时间
    AutoTaskLogLevel level{ AutoTaskLogLevel::Info };  // 提示信息 警告信息 错误信息
    QString userID;                                    // 操作用户ID
    QString context;                                   // 任务日志具体内容

    AutoTaskLogData() { qRegisterMetaType<AutoTaskLogData>("AutoTaskLogData"); }

    AutoTaskLogData(const QString& uuid, const QString& taskCode, const QString& serialNumber, const QString& context, AutoTaskLogLevel level)
    {
        this->uuid = uuid;
        this->taskCode = taskCode;
        this->serialNumber = serialNumber;
        this->context = context;
        this->level = level;
        this->createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    }
};

struct AutoTaskLogDataCondition
{
    QString uuid;
    quint64 startTime;
    quint64 endTime;
};

using AutoTaskLogDataList = QList<AutoTaskLogData>;

class AutoTaskLogHelper
{
public:
    static QString logLevelToString(const AutoTaskLogLevel& level)
    {
        switch (level)
        {
        case AutoTaskLogLevel::Info: return QString("提示");
        case AutoTaskLogLevel::Warning: return QString("警告");
        case AutoTaskLogLevel::Error: return QString("错误");
        case AutoTaskLogLevel::SpecificTips: return QString("关键提示");
        default: return QString("未知");
        }
    }

    static QColor logLevelToColor(const AutoTaskLogLevel& level)
    {
        switch (level)
        {
        case AutoTaskLogLevel::Info:
        {
            return QColor(Qt::black);
        }
        case AutoTaskLogLevel::Warning:
        {
            return QColor(229, 153, 0);
        }
        case AutoTaskLogLevel::Error:
        {
            return QColor(231, 35, 35);
        }
        case AutoTaskLogLevel::SpecificTips:
        {
            return QColor(0, 170, 0);
        }
        }
        return QColor(Qt::black);
    }
};

#endif  // AUTORUNTASKLOGMESSAGEDEFINE_H
