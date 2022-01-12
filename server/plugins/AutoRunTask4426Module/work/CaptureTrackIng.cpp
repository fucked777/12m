#include "CaptureTrackIng.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include <QDebug>
#include <QThread>
// int CaptureTrackIng::typeId = qRegisterMetaType<CaptureTrackIng>();
CaptureTrackIng::CaptureTrackIng(QObject* parent)
    : BaseEvent(parent)
{
}

CaptureTrackIng::~CaptureTrackIng() {}

void CaptureTrackIng::doSomething()
{
    taskMachine->updateStatus("CaptureTrackIng", TaskStepStatus::Running);
    // auto taskPlan = taskMachine->taskPlan();
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);

    auto currentDateTime = GlobalData::currentDateTime();
    auto msg = QString("距离跟踪开始还有%1秒开始执行").arg(currentDateTime.secsTo(taskTimeList.m_track_start_time));
    notifyInfoLog(msg);
    while (taskTimeList.m_track_start_time > currentDateTime)
    {
        if (isExit())
        {
            return;
        }
        QThread::msleep(800);
        currentDateTime = GlobalData::currentDateTime();
    };

    notifyInfoLog("开始自跟踪");
    VoiceAlarmPublish::publish("跟踪开始");

    //校相完成后给ACU下发任务开始命令
    QMap<QString, QVariant> acuStartTaskMap;
    acuStartTaskMap["TaskIdent"] = taskTimeList.manualMessage.masterTaskBz;
    m_singleCommandHelper.packSingleCommand("Step_ACU_TASK_START", m_packCommand, taskTimeList.onlineACU, acuStartTaskMap);
    waitExecSuccess(m_packCommand);

    //QThread::sleep(1);
    taskMachine->updateStatus("CaptureTrackIng", TaskStepStatus::Finish);
    notify("GoUp");
}
