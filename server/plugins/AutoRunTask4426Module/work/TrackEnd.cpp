#include "TrackEnd.h"
#include "BaseHandler.h"
#include "ControlFlowHandler.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "TaskGlobalInfo.h"

// int TrackEnd::typeId = qRegisterMetaType<TrackEnd>();
TrackEnd::TrackEnd(QObject* parent)
    : BaseEvent(parent)
{
}

void TrackEnd::doSomething()
{
    taskMachine->updateStatus("TrackEnd", TaskStepStatus::Running);
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);
    // QMap<int, DeviceID> m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

    // PackCommand packCommand;
    /***
     *  T6 跟踪结束时间
     * 系统监控控制ACU、测控基带、数传基带、DTE送数关和存盘关
     ***/
    auto trackEndTime = taskTimeList.m_track_end_time;
    auto currentDateTime = GlobalData::currentDateTime();
    int number = -1;
    QString info = QString("跟踪结束流程还有%1秒开始执行").arg(currentDateTime.secsTo(trackEndTime));
    notifyInfoLog(info);
    while (trackEndTime > currentDateTime)
    {
        // 如果退出就直接返回
        if (isExit())
        {
            return;
        }

        number++;

        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
    }
    VoiceAlarmPublish::publish("跟踪结束");
    notifyInfoLog("跟踪结束流程开始执行");

    ControlFlowHandler controlFlowHandler;
    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &BaseEvent::signalSendToDevice);
    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &BaseEvent::notifyInfoLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &BaseEvent::notifyWarningLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &BaseEvent::notifyErrorLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalSpecificTipsMsg, this, &BaseEvent::notifySpecificTipsLog);

    auto bak = taskTimeList.manualMessage;
    bak.manualType = ManualType::ResourceRelease;
    controlFlowHandler.setRunningFlag(m_runningFlag);
    controlFlowHandler.handle(bak, false);

    /* 任务结束是否转收藏 */
    auto taskEndACUCollection = GlobalData::getAutorunPolicyData("TaskEndACUCollection", "false").toBool();
    if (taskEndACUCollection)
    {
        /* 任务结束 */
        //m_singleCommandHelper.packSingleCommand("Step_ACU_TASK_STOP", m_packCommand, taskTimeList.onlineACU);
        //waitExecSuccess(m_packCommand);

        /* 人工运行 */
        //m_singleCommandHelper.packSingleCommand("Step_ACU_OperatMode_Artificial", m_packCommand, taskTimeList.onlineACU);
        //waitExecSuccess(m_packCommand);

        /* 待机 */
        //m_singleCommandHelper.packSingleCommand("Step_ACU_DaiJI", m_packCommand, taskTimeList.onlineACU);
        //waitExecSuccess(m_packCommand);

        m_singleCommandHelper.packSingleCommand("Step_ACU_SHOUCANG", m_packCommand, taskTimeList.onlineACU);
        waitExecSuccess(m_packCommand);
    }

    notifyInfoLog("跟踪结束流程完成");

    auto taskEndTime = taskTimeList.m_task_end_time;
    currentDateTime = GlobalData::currentDateTime();
    info = QString("任务结束流程还有%1秒开始执行").arg(currentDateTime.secsTo(taskEndTime));
    notifyInfoLog(info);
    while (taskEndTime > currentDateTime)
    {
        if (isExit())
        {
            return;
        }
        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
    }

    notifyInfoLog("任务结束流程开始执行");

    /* 待机 */
    m_singleCommandHelper.packSingleCommand("Step_ACU_DaiJI", m_packCommand, taskTimeList.onlineACU);
    waitExecSuccess(m_packCommand);
    /* 资源释放 */
    //    ControlFlowHandler controlFlowHandler;
    //    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &BaseEvent::signalSendToDevice);
    //    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &BaseEvent::notifyInfoLog);
    //    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &BaseEvent::notifyWarningLog);
    //    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &BaseEvent::notifyErrorLog);
    //    connect(&controlFlowHandler, &ControlFlowHandler::signalSpecificTipsMsg, this, &BaseEvent::notifySpecificTipsLog);

    //    auto bak = taskTimeList.manualMessage;
    //    bak.manualType = ManualType::ResourceRelease;
    //    controlFlowHandler.setRunningFlag(m_runningFlag);
    //    controlFlowHandler.handle(bak, false);

    VoiceAlarmPublish::publish("任务结束");
    notifyInfoLog("任务结束流程完成");
    taskMachine->updateStatus("TrackEnd", TaskStepStatus::Finish);
    notify("finish");
}
