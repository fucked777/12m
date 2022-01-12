#ifndef MESSAGEPUBLISH_H
#define MESSAGEPUBLISH_H

#include <QString>

#include "RedisHelper.h"

// 控制命令响应通道
#define ControlCmdResponseChannel "ControlCmdResponseChannel"

// 系统日志推送通道
#define SystemLogChannel "SystemLogChannel"
// 任务自动化运行日志推送通道
#define AutoRunTaskLogChannel "AutoRunTaskLogChannel"
// 任务自动化运行流程状态推送通道
#define AutoRunTaskStepStatusChannel "AutoRunTaskStepStatusChannel"
// 设备状态数据推送通道
#define DeviceStatusDataChannel "DeviceStatusDataChannel"

// 自动化测试信息推送通道
#define AutoTestDataChannel "AutoTestDataChannel"

struct ControlCmdResponse;

struct CurrentRunningTaskPlanData;

// 系统日志推送
class SystemLogPublish
{
public:
    static void infoMsg(const QString& context);
    static void warningMsg(const QString& context);
    static void errorMsg(const QString& context);
    static void specificTipsMsg(const QString& context);

    static void infoMsg(const QString& context, const QString& userID, const QString& moduleName);
    static void warningMsg(const QString& context, const QString& userID, const QString& moduleName);
    static void errorMsg(const QString& context, const QString& userID, const QString& moduleName);
    static void specificTipsMsg(const QString& context, const QString& userID, const QString& moduleName);

    static void infoMsg(const QString& context, const QString& moduleName);
    static void warningMsg(const QString& context, const QString& moduleName);
    static void errorMsg(const QString& context, const QString& moduleName);
    static void specificTipsMsg(const QString& context, const QString& moduleName);
};

// 任务自动化运行日志推送
class AutoTaskLogPublish
{
public:
    static void infoMsg(const QString& uuid, const QString& taskCode, const QString& context);
    static void warningMsg(const QString& uuid, const QString& taskCode, const QString& context);
    static void errorMsg(const QString& uuid, const QString& taskCode, const QString& context);
    static void specificTipsMsg(const QString& uuid, const QString& taskCode, const QString& context);
};

// 任务自动化运行流程状态推送
class AutoRunTaskStepStatusPublish
{
public:
    static void publish(const CurrentRunningTaskPlanData& runningTaskPlanData);
    static CurrentRunningTaskPlanData getRunningTask();
    static void clearRunningTask();
};

// 控制命令响应推送
class ControlCmdResponsePublish
{
public:
    static void publish(const ControlCmdResponse& controlCmdResponse);
};

// 语音告警推送
class VoiceAlarmPublish
{
public:
    static void publish(const QString& msg);
};

#endif  // MESSAGEPUBLISH_H
