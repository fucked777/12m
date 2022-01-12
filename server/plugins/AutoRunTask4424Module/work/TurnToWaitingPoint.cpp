#include "TurnToWaitingPoint.h"

#include <QApplication>
#include <QDebug>
#include <QThread>

#include "MessagePublish.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
TurnToWaitingPoint::TurnToWaitingPoint(QObject* parent)
    : BaseEvent(parent)
{
}

void TurnToWaitingPoint::doSomething()
{
    // 加载需要的数据
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);

    notifyInfoLog("转等待点流程开始执行");
    VoiceAlarmPublish::publish("系统转等待点");

    notifyInfoLog(QString("控制ACU进入程序引导"));

    /* 转等待点 */
    m_singleCommandHelper.packSingleCommand("Step_ACU_TurnWaitingPoint", m_packCommand, taskTimeList.onlineACU);
    waitExecSuccess(m_packCommand);

    // /* 控制ACU进入程序引导 */
    // m_singleCommandHelper.packSingleCommand("Step_ACU_ProgramGuide", m_packCommand, taskTimeList.onlineACU);
    // waitExecSuccess(m_packCommand);

    notifyInfoLog(QString("转等待点流程完成"));
    auto traceStartDateTime = taskTimeList.m_track_start_time;
    auto taskStartDatetime = taskTimeList.m_task_start_time;
    QDateTime currentDateTime = GlobalData::currentDateTime();
    QString info = QString("校相流程还有%1秒开始执行").arg(currentDateTime.secsTo(traceStartDateTime));
    notifyInfoLog(info);

    while (taskStartDatetime >= currentDateTime)
    {
        // 如果退出就直接返回
        if (m_isExit)
            return;
        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
        QApplication::processEvents();  //让出当前线程  run线程和函数线程不在同一个线程里面，让出线程可以运行外部进来的信号。
    }

    notifyInfoLog(QString("转等待点流程完成"));

    notify("TaskStart");
}
