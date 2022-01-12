#include "TurnToWaitingPoint.h"

#include <QApplication>
#include <QDebug>
#include <QThread>

#include "DeviceProcessMessageSerialize.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
// int TurnToWaitingPoint::typeId = qRegisterMetaType<TurnToWaitingPoint>();
TurnToWaitingPoint::TurnToWaitingPoint(QObject* parent)
    : BaseEvent(parent)
{
}

void TurnToWaitingPoint::doSomething()
{
    taskMachine->updateStatus("TurnToWaitingPoint", TaskStepStatus::Running);

    VoiceAlarmPublish::publish("转等待点");

    // 加载需要的数据
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);

    notifyInfoLog("转等待点流程开始执行");
    /* 获取ACU当前任务信息 */
    auto result = waitTaskPlanInfo(taskTimeList);

    if (m_apprPosit < 0 || m_pitchPitch < 0)
    {
        VoiceAlarmPublish::publish("轨根异常");
    }

    if (result)
    {
        waitAngle(taskTimeList);
    }
//    else
//    {
        VoiceAlarmPublish::publish("系统程引开始");
        /* 控制ACU进入程序引导 */
        m_singleCommandHelper.packSingleCommand("Step_ACU_ProgramGuide", m_packCommand, taskTimeList.onlineACU);
        waitExecSuccess(m_packCommand);
//    }

    VoiceAlarmPublish::publish("转等待点成功");
    notifyInfoLog(QString("转等待点流程结束"));
    auto traceStartDateTime = taskTimeList.m_track_start_time;
    auto taskStartDatetime = taskTimeList.m_task_start_time;
    QDateTime currentDateTime = GlobalData::currentDateTime();
    auto info = QString("校相流程还有%1秒开始执行").arg(currentDateTime.secsTo(traceStartDateTime));
    notifyInfoLog(info);

    while (taskStartDatetime >= currentDateTime)
    {
        // 如果退出就直接返回
        if (isExit())
        {
            return;
        }
        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
        QApplication::processEvents();  //让出当前线程  run线程和函数线程不在同一个线程里面，让出线程可以运行外部进来的信号。
    }
    taskMachine->updateStatus("TurnToWaitingPoint", TaskStepStatus::Finish);
    notify("TaskStart");
}
void TurnToWaitingPoint::waitAngle(const TaskTimeList& taskTimeList)
{
    VoiceAlarmPublish::publish("系统转等待点");
    /* 转等待点 */
    m_singleCommandHelper.packSingleCommand("Step_ACU_WAITING", m_packCommand, taskTimeList.onlineACU);
    waitExecSuccess(m_packCommand);

    QStringList keys;
    keys << "CurrentAZ"
         << "CurrentPA";

    int timeOut = GlobalData::getCmdTimeCount(30);
    for (auto i = 0; i < timeOut; ++i)
    {
        //实时获取ACU的值 当前方位俯仰
        auto currentAZPA = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, keys);
        if (fabs(m_azimWaitPoint - currentAZPA["CurrentAZ"].toDouble()) < 1 && fabs(m_pitchWaitPoint - currentAZPA["CurrentPA"].toDouble()) < 1)
        {
            notifyInfoLog("转等待点完成");
            return;
        }
        GlobalData::waitCmdTimeMS();
    }

    notifyWarningLog("转等待点等待超时");
}
bool TurnToWaitingPoint::waitTaskPlanInfo(const TaskTimeList& taskTimeList)
{
    QVariantMap replace;
    replace["TaskIdent"] = taskTimeList.manualMessage.masterTaskBz;
    m_singleCommandHelper.packSingleCommand("Step_ACU_TaskPlanQuery", m_packCommand, taskTimeList.onlineACU, replace);
    if (!waitExecSuccess(m_packCommand))
    {
        return false;
    }

    int timeOut = GlobalData::getCmdTimeCount(10);
    auto cacheKey = DeviceProcessHelper::getCmdResultRedisKey(m_packCommand.deviceID, m_packCommand.cmdId);
    while (timeOut--)
    {
        if (isExit())
        {
            return false;
        }
        QString json;
        if (RedisHelper::getInstance().getData(cacheKey, json) && !json.isEmpty())
        {
            CmdResult result;
            json >> result;

            m_apprPosit = result.m_paramdataMap.value("ApprPosit").toDouble();
            m_pitchPitch = result.m_paramdataMap.value("PitchPitch").toDouble();
            m_azimWaitPoint = result.m_paramdataMap.value("AzimWaitPoint").toDouble();
            m_pitchWaitPoint = result.m_paramdataMap.value("PitchWaitPoint").toDouble();
            auto msg = QString("当前进站方位:%1 进站俯仰:%2 等待点方位:%3 等待点俯仰:%4")
                           .arg(m_apprPosit, 0, 'f')
                           .arg(m_pitchPitch, 0, 'f')
                           .arg(m_azimWaitPoint, 0, 'f')
                           .arg(m_pitchWaitPoint, 0, 'f');
            notifyInfoLog(msg);
            return true;
        }
        GlobalData::waitCmdTimeMS();
    }

    m_apprPosit = -1.0;
    m_pitchPitch = -1.0;
    m_azimWaitPoint = -1.0;
    m_pitchWaitPoint = -1.0;
    auto msg = QString("查询当前ACU任务计划信息超时");
    notifyErrorLog(msg);
    return false;
}
