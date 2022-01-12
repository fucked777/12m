#include "Calibration.h"
#include "BaseEvent.h"
#include "BaseHandler.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "OnekeyXXHandler.h"
#include "PhaseCalibrationSerialize.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskRunCommonHelper.h"
#include <QApplication>
#include <QDebug>
#include <QThread>

// int Calibration::typeId = qRegisterMetaType<Calibration>();
Calibration::Calibration(QObject* parent)
    : BaseEvent(parent)
{
}

void Calibration::doSomething()
{
    taskMachine->updateStatus("Calibration", TaskStepStatus::Running);
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);


    auto track_start_time = taskTimeList.m_track_start_time;
    auto currentDateTime = GlobalData::currentDateTime();
    while (track_start_time > currentDateTime)
    {
        // 如果退出就直接返回
        if (isExit())
        {
            return;
        }

        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
    }

    if (!taskTimeList.m_calibration)
    {
        notifyInfoLog(QString("当前任务配置为不校相，跳过校相流程"));
        taskMachine->updateStatus("Calibration", TaskStepStatus::Continue);
        VoiceAlarmPublish::publish("跳过校相");
        notify("CaptureTrackIng");
        return;
    }

    notifySpecificTipsLog("校相处理流程开始执行");
    VoiceAlarmPublish::publish("校相开始");

    OnekeyXXHandler onekeyXXHandler;
    connect(&onekeyXXHandler, &BaseHandler::signalSendToDevice, this, &Calibration::signalSendToDevice);
    connect(&onekeyXXHandler, &BaseHandler::signalInfoMsg, this, &Calibration::notifyInfoLog);        //提示日志
    connect(&onekeyXXHandler, &BaseHandler::signalWarningMsg, this, &Calibration::notifyWarningLog);  //警告日志
    connect(&onekeyXXHandler, &BaseHandler::signalErrorMsg, this, &Calibration::notifyErrorLog);      //错误日志
    connect(&onekeyXXHandler, &BaseHandler::signalSpecificTipsMsg, this, &Calibration::notifySpecificTipsLog);
    connect(&onekeyXXHandler, &OnekeyXXHandler::sg_audio, [](const QString& msg) { VoiceAlarmPublish::publish(msg); });

    onekeyXXHandler.setRunningFlag(m_runningFlag);
    onekeyXXHandler.setManualMessage(taskTimeList.manualMessage);

    auto isSuccess = onekeyXXHandler.handle();

    notifySpecificTipsLog("校相处理流程完成");

    if (isSuccess)
    {
        VoiceAlarmPublish::publish("校相成功");
    }
    else
    {
        VoiceAlarmPublish::publish("校相失败");
    }

    VoiceAlarmPublish::publish("跟踪开始");

    /* 是标校任务就到这里就任务结束 */
    if (taskTimeList.taskPlanData.deviceWorkTask.m_working_mode == QString("BJ"))
    {
        /* 处理一下标校任务将计划跟踪结束时间和任务结束时间都放到当前时间 */
        taskTimeList.m_track_end_time = GlobalData::currentDateTime();
        taskTimeList.m_task_end_time = taskTimeList.m_track_end_time.addSecs(60);
        TaskGlobalInfoSingleton::getInstance().setTaskTimeList(uuid, taskTimeList);

        notifySpecificTipsLog("标校任务结束");
        notify("TrackEnd");
        return;
    }
    if (isSuccess)
    {
        taskMachine->updateStatus("Calibration", TaskStepStatus::Finish);
        notify("CaptureTrackIng");
    }
    else
    {
        taskMachine->updateStatus("Calibration", TaskStepStatus::Error);
        notify("CaptureTrackIng");
    }
}
