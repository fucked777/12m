#include "MessagePublish.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "AutoRunTaskMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "PlanRunMessageDefine.h"
#include "SystemLogMessageSerialize.h"
#include "TaskPlanMessageSerialize.h"

void SystemLogPublish::infoMsg(const QString& context)
{
    SystemLogData systemLog(context, LogLevel::Info);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}

void SystemLogPublish::warningMsg(const QString& context)
{
    SystemLogData systemLog(context, LogLevel::Warning);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}

void SystemLogPublish::errorMsg(const QString& context)
{
    SystemLogData systemLog(context, LogLevel::Error);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}
void SystemLogPublish::specificTipsMsg(const QString& context)
{
    SystemLogData systemLog(context, LogLevel::SpecificTips);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}

void SystemLogPublish::infoMsg(const QString& context, const QString& userID, const QString& moduleName)
{
    SystemLogData systemLog(context, userID, moduleName, LogLevel::Info);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}

void SystemLogPublish::warningMsg(const QString& context, const QString& userID, const QString& moduleName)
{
    SystemLogData systemLog(context, userID, moduleName, LogLevel::Warning);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}

void SystemLogPublish::errorMsg(const QString& context, const QString& userID, const QString& moduleName)
{
    SystemLogData systemLog(context, userID, moduleName, LogLevel::Error);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}
void SystemLogPublish::specificTipsMsg(const QString& context, const QString& userID, const QString& moduleName)
{
    SystemLogData systemLog(context, userID, moduleName, LogLevel::SpecificTips);
    QString json;
    json << systemLog;
    RedisHelper::getInstance().publish(SystemLogChannel, json);
}

void SystemLogPublish::infoMsg(const QString& context, const QString& moduleName) { infoMsg(context, "System", moduleName); }
void SystemLogPublish::warningMsg(const QString& context, const QString& moduleName) { warningMsg(context, "System", moduleName); }
void SystemLogPublish::errorMsg(const QString& context, const QString& moduleName) { errorMsg(context, "System", moduleName); }
void SystemLogPublish::specificTipsMsg(const QString& context, const QString& moduleName) { specificTipsMsg(context, "System", moduleName); }

void AutoTaskLogPublish::infoMsg(const QString& uuid, const QString& taskCode, const QString& context)
{
    AutoTaskLogData autoTasklog;
    autoTasklog.taskCode = taskCode;
    autoTasklog.uuid = uuid;
    autoTasklog.userID = "";
    autoTasklog.context = context;
    autoTasklog.createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    autoTasklog.level = AutoTaskLogLevel::Info;

    QString autoTaskJson;
    autoTaskJson << autoTasklog;
    RedisHelper::getInstance().publish(AutoRunTaskLogChannel, autoTaskJson);
}

void AutoTaskLogPublish::warningMsg(const QString& uuid, const QString& taskCode, const QString& context)
{
    AutoTaskLogData autoTasklog;
    autoTasklog.taskCode = taskCode;
    autoTasklog.uuid = uuid;
    autoTasklog.userID = "";
    autoTasklog.context = context;
    autoTasklog.createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    autoTasklog.level = AutoTaskLogLevel::Warning;

    QString autoTaskJson;
    autoTaskJson << autoTasklog;
    RedisHelper::getInstance().publish(AutoRunTaskLogChannel, autoTaskJson);
}

void AutoTaskLogPublish::errorMsg(const QString& uuid, const QString& taskCode, const QString& context)
{
    AutoTaskLogData autoTasklog;
    autoTasklog.taskCode = taskCode;
    autoTasklog.uuid = uuid;
    autoTasklog.userID = "";
    autoTasklog.context = context;
    autoTasklog.createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    autoTasklog.level = AutoTaskLogLevel::Error;

    QString autoTaskJson;
    autoTaskJson << autoTasklog;
    RedisHelper::getInstance().publish(AutoRunTaskLogChannel, autoTaskJson);
}

void AutoTaskLogPublish::specificTipsMsg(const QString& uuid, const QString& taskCode, const QString& context)
{
    AutoTaskLogData autoTasklog;
    autoTasklog.taskCode = taskCode;
    autoTasklog.uuid = uuid;
    autoTasklog.userID = "";
    autoTasklog.context = context;
    autoTasklog.createTime = GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT);
    autoTasklog.level = AutoTaskLogLevel::SpecificTips;

    QString autoTaskJson;
    autoTaskJson << autoTasklog;
    RedisHelper::getInstance().publish(AutoRunTaskLogChannel, autoTaskJson);
}

void AutoRunTaskStepStatusPublish::publish(const CurrentRunningTaskPlanData& runningTaskPlanData)
{
    auto temp = runningTaskPlanData;
    // 从全局获取手动控制流程的信息，一起推送
    temp.msg = GlobalData::getTaskRunningInfo();

    QString json;
    json << temp;
    RedisHelper::getInstance().publish(AutoRunTaskStepStatusChannel, json);
    RedisHelper::getInstance().setData(AutoRunTaskStepStatusChannel, json, 180);
}
CurrentRunningTaskPlanData AutoRunTaskStepStatusPublish::getRunningTask()
{
    QString json;
    RedisHelper::getInstance().getData(AutoRunTaskStepStatusChannel, json);

    CurrentRunningTaskPlanData tempData;
    json >> tempData;
    return tempData;
}
void AutoRunTaskStepStatusPublish::clearRunningTask()
{
    RedisHelper::getInstance().setData(AutoRunTaskStepStatusChannel, QString());
}

void ControlCmdResponsePublish::publish(const ControlCmdResponse& controlCmdResponse)
{
    QString json;
    json << controlCmdResponse;
    RedisHelper::getInstance().publish(ControlCmdResponseChannel, json);
}

void VoiceAlarmPublish::publish(const QString& msg) { RedisHelper::getInstance().publistMusic(msg); }
