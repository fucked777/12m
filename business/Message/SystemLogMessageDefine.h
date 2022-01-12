#ifndef SYSTEMLOGMESSAGEDEFINE_H
#define SYSTEMLOGMESSAGEDEFINE_H

#include <QDateTime>
#include <QString>
#include <QUuid>

#include "GlobalData.h"
#include "GlobalDefine.h"

enum class LogLevel
{
    Info = 0,      // 信息
    Warning,       // 警告
    Error,         // 错误
    SpecificTips,  // 特殊提示
};

enum class LogType
{
    User,   // 用户操作日志
    System  // 系统日志
};

struct SystemLogData
{
    SystemLogData() { qRegisterMetaType<SystemLogData>("SystemLogData"); }

    SystemLogData(const QString& logContext, LogLevel level, LogType type = LogType::System)
    {
        this->id = QUuid::createUuid().toString();
        this->level = level;
        this->type = type;
        this->context = logContext;
        this->userID = QString("Default");
        this->module = "";
        this->createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    }

    SystemLogData(const QString& logContext, const QString& userId, const QString& moduleName, LogLevel level, LogType type = LogType::System)
    {
        this->id = QUuid::createUuid().toString();
        this->level = level;
        this->type = type;
        this->context = logContext;
        this->userID = userId;
        this->module = moduleName;
        this->createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    }

    QString id;                        // 日志唯一ID
    LogLevel level{ LogLevel::Info };  // 日志等级
    LogType type{ LogType::System };   // 日志类型
    QString context;                   // 日志信息
    QString userID{ "Default" };       // 当前用户操作ID
    QString module;                    // 日志产生模块
    QString createTime;                // 日志时间
};

struct SystemLogCondition
{
    SystemLogCondition()
    {
        startTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
        endTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    }
    SystemLogCondition(QDateTime start, QDateTime end, LogLevel level, int currentPage = -1, int pageSize = -1)
    {
        this->startTime = start.toString(DATETIME_ORIGIN_FORMAT);
        this->endTime = end.toString(DATETIME_ORIGIN_FORMAT);
        this->level = level;
        this->currentPage = currentPage;
        this->pageSize = pageSize;
    }

    QString startTime;
    QString endTime;
    LogLevel level{ LogLevel::Info };  // 日志等级
    int currentPage{ 0 };
    int pageSize{ 0 };
};

struct SystemLogACK
{
    SystemLogACK()
    {
        this->totalCount = 0;
        this->currentPage = 0;
        dataList.clear();
    }

    int totalCount{ 0 };
    int currentPage{ 0 };

    QList<SystemLogData> dataList;
};

#endif  // SYSTEMLOGMESSAGEDEFINE_H
