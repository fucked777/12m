#include "TaskMachine.h"

#include <QApplication>
#include <QDebug>
#include <QMetaType>
#include <QUuid>
#include <QMutex>
#include <QMutexLocker>

#include "AutoRunTaskLogMessageSerialize.h"
#include "BaseEvent.h"
#include "BaseStatus.h"
#include "Calibration.h"
#include "CaptureTrackIng.h"
#include "GoUp.h"
#include "MessagePublish.h"
#include "PostProcessing.h"
#include "RedisHelper.h"
#include "SingleCommandHelper.h"
#include "Start.h"
#include "TaskGlobalInfo.h"
#include "TaskPlanMessageDefine.h"
#include "TaskPlanSql.h"
#include "TaskPreparation.h"
#include "TaskStart.h"
#include "TrackEnd.h"
#include "TurnToWaitingPoint.h"
#include "ZeroCorrection.h"

TaskMachine::TaskMachine(QObject* parent)
    : QThread(parent)
{
    VoiceAlarmPublish::publish("自动化运行开始");
    init();
    m_machineUUID = QUuid::createUuid().toString();
}

TaskMachine::~TaskMachine()
{
    stopRing();
}
void TaskMachine::stopRing()
{
    mIsRunning = false;

    qWarning() << "任务虚拟机停止中";
    quit();
    while (isRunning() && !isFinished())
    {
        QApplication::processEvents();
        QThread::msleep(10);
    }
    wait();
    qWarning() << "任务虚拟机停止完成";

    qWarning() << "任务虚拟机任务流程停止中";
    if (m_currentEvent != nullptr)
    {
        //停止下发的任务
        //auto taskTimelist = TaskGlobalInfoSingleton::getInstance().taskTimeList(m_machineUUID);
        m_workThread.disconnect();
        notifyInfoLog("任务开始停止");
        m_workThread.quit();
        while (m_workThread.isRunning() && !m_workThread.isFinished())
        {
            QApplication::processEvents();
            QThread::msleep(10);
        }
        m_workThread.wait();
        notifyInfoLog("任务停止完成");

        delete m_currentEvent;
        m_currentEvent = nullptr;
    }
    qWarning() << "任务虚拟机任务流程停止完成";
}
void TaskMachine::restoreTaskRuning(TaskStep taskStep, const QString& uuid, const QMap<TaskStep, TaskStepStatus>&statusMap)
{
    mStepStatusMap = statusMap;
    m_machineUUID = uuid;
    m_restoreSleepTime = 3;
    if (taskStep == TaskStep::START)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "start";
        start();
    }
    else if (taskStep == TaskStep::TASK_PREPARATION)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "TaskPreparation";
        start();
    }
    else if (taskStep == TaskStep::ZERO_CORRECTION)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "ZeroCorrection";
        start();
    }
    else if (taskStep == TaskStep::TASK_START)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "TaskStart";
        start();
    }
    else if (taskStep == TaskStep::TURN_TOWAITINGPOINT)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "TurnToWaitingPoint";
        start();
    }
    else if (taskStep == TaskStep::CALIBRATION)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "Calibration";
        start();
    }
    else if (taskStep == TaskStep::CAPTURE_TRACKLING)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "CaptureTrackIng";
        start();
    }
    else if (taskStep == TaskStep::GO_UP)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "GoUp";
        start();
    }
    else if (taskStep == TaskStep::TRACK_END)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "TrackEnd";
        start();
    }
    else if (taskStep == TaskStep::POST_PROCESSING)
    {
        mCurrentRunningStatus.clear();
        m_currentStatus = "PostProcessing";
        start();
    }
}

void TaskMachine::run()
{
    qWarning() << "任务虚拟机线程启动";
    mIsRunning = true;

    if(m_restoreSleepTime > 0)
    {
        QThread::sleep(m_restoreSleepTime);
    }
    while (mIsRunning)
    {
        //QMutexLocker locker(&m_Mutex);
        QList<BaseStatus> values = m_currentStatusMap.values(m_currentStatus);
        int countSize = values.size();

        if (mCurrentRunningStatus != m_currentStatus)
        {
            if (m_currentStatus == "start")
            {
                if (countSize > 0)
                {
                    QString typeString = values.first().currentClass;
                    m_currentEvent = instance(typeString);
                    m_currentEvent->setRunningFlag(&mIsRunning);
                    m_currentEvent->registerMachine(this);
                    m_currentEvent->moveToThread(&m_workThread);
                    //connect(m_currentEvent, &BaseEvent::notifyMachine, this, &TaskMachine::acceptNotify);

                    mCurrentRunningStatus = m_currentStatus;
                    if (!m_workThread.isRunning())
                    {
                        m_workThread.start();
                    }
                    //推送当前开始的状态
                    updateStatus(mCurrentRunningStatus, TaskStepStatus::Start);

                    emit m_currentEvent->start();
                }
                else
                {
                    notifyErrorLog("流程错误");
                    mIsRunning = false;
                }
            }
            else if (m_currentStatus == "finish")
            {
                notifyInfoLog("任务完成");
                mIsRunning = false;
            }
            else if (m_currentStatus == "exception")
            {
                notifyErrorLog("任务异常");
                //                mIsRunning = false;
            }
            else if (m_currentStatus == "error")
            {
                notifyErrorLog("任务错误");
                mIsRunning = false;
            }
            else
            {
                if (values.size() > 0)
                {
                    QString typeString = values.first().currentClass;

                    m_currentEvent = instance(typeString);
                    m_currentEvent->setRunningFlag(&mIsRunning);
                    m_currentEvent->registerMachine(this);
                    m_currentEvent->moveToThread(&m_workThread);
                    //connect(m_currentEvent, &BaseEvent::notifyMachine, this, &TaskMachine::acceptNotify);
                    connect(m_currentEvent, &BaseEvent::signalSendToDevice, this, &TaskMachine::signalSendToDevice);

                    if (!m_workThread.isRunning())
                    {
                        m_workThread.start();
                    }
                    mCurrentRunningStatus = m_currentStatus;
                    //推送当前开始的状态
                    updateStatus(mCurrentRunningStatus, TaskStepStatus::Start);
                    emit m_currentEvent->start();
                }
            }
        }

        if(m_currentEvent != nullptr)
        {
            auto nextStatus = m_currentEvent->nextStatus();
            if(!nextStatus.isEmpty())
            {
                acceptNotify(nextStatus);
            }
        }
        QThread::msleep(200);

        QApplication::processEvents();
    }

    m_restoreSleepTime=0;
    qWarning() << "任务虚拟机线程退出完成";
}

void TaskMachine::notifyInfoLog(const QString& logStr)
{
    AutoTaskLogPublish::infoMsg(mTaskPlanData.deviceWorkTask.m_uuid, mTaskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::infoMsg(logStr);
}

void TaskMachine::notifyWarningLog(const QString& logStr)
{
    AutoTaskLogPublish::warningMsg(mTaskPlanData.deviceWorkTask.m_uuid, mTaskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::warningMsg(logStr);
}

void TaskMachine::notifyErrorLog(const QString& logStr)
{
    AutoTaskLogPublish::errorMsg(mTaskPlanData.deviceWorkTask.m_uuid, mTaskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::errorMsg(logStr);
}
void TaskMachine::notifySpecificTipsLog(const QString& logStr)
{
    AutoTaskLogPublish::specificTipsMsg(mTaskPlanData.deviceWorkTask.m_uuid, mTaskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::specificTipsMsg(logStr);
}

void TaskMachine::updateStatus(const QString& stepName, TaskStepStatus status)
{
    TaskStep step = TaskStep::UNKONW;
    if (stepName == "start")
        step = TaskStep::START;
    if (stepName == "TaskPreparation")
        step = TaskStep::TASK_PREPARATION;
    if (stepName == "ZeroCorrection")
        step = TaskStep::ZERO_CORRECTION;
    if (stepName == "TaskStart")
        step = TaskStep::TASK_START;
    if (stepName == "TurnToWaitingPoint")
        step = TaskStep::TURN_TOWAITINGPOINT;
    if (stepName == "Calibration")
        step = TaskStep::CALIBRATION;
    if (stepName == "CaptureTrackIng")
        step = TaskStep::CAPTURE_TRACKLING;
    if (stepName == "GoUp")
        step = TaskStep::GO_UP;
    if (stepName == "TrackEnd")
        step = TaskStep::POST_PROCESSING;
    if (stepName == "PostProcessing")
        step = TaskStep::POST_PROCESSING;

    if (step == TaskStep::UNKONW)
    {
        return;
    }
    mStepStatusMap[step] = status;
}

QMap<TaskStep, TaskStepStatus> TaskMachine::getTaskStepStatus() const { return mStepStatusMap; }

TaskPlanData TaskMachine::taskPlan() const { return mTaskPlanData; }
void TaskMachine::setTaskPlan(const TaskPlanData& taskPlanData) { mTaskPlanData = taskPlanData; }
QString TaskMachine::curTaskDevUuid() const { return mTaskPlanData.deviceWorkTask.m_uuid; }

//QString TaskMachine::currentStatus() { return m_currentStatus; }

void TaskMachine::init()
{
    // 初始化所有流程步骤都为未开始
    mStepStatusMap[TaskStep::START] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::TASK_PREPARATION] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::ZERO_CORRECTION] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::TASK_START] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::TURN_TOWAITINGPOINT] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::CALIBRATION] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::CAPTURE_TRACKLING] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::GO_UP] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::TRACK_END] = TaskStepStatus::NoStart;
    mStepStatusMap[TaskStep::POST_PROCESSING] = TaskStepStatus::NoStart;

    //任务开始就开始判断任务状态
    m_currentStatusMap.insertMulti("start", BaseStatus("start", "Start", "TaskPreparation"));
    m_currentStatusMap.insertMulti("start", BaseStatus("start", "Start", "exception"));
    m_currentStatusMap.insertMulti("start", BaseStatus("start", "Start", "finish"));
    m_currentStatusMap.insertMulti("start", BaseStatus("start", "Start", "error"));

    //任务准备    配置链路  任务参数宏下发
    m_currentStatusMap.insertMulti("TaskPreparation", BaseStatus("TaskPreparation", "TaskPreparation", "ZeroCorrection"));
    m_currentStatusMap.insertMulti("TaskPreparation", BaseStatus("TaskPreparation", "TaskPreparation", "exception"));
    m_currentStatusMap.insertMulti("TaskPreparation", BaseStatus("TaskPreparation", "TaskPreparation", "finish"));
    m_currentStatusMap.insertMulti("TaskPreparation", BaseStatus("TaskPreparation", "TaskPreparation", "error"));

    //校零处理
    m_currentStatusMap.insertMulti("ZeroCorrection", BaseStatus("ZeroCorrection", "ZeroCorrection", "TurnToWaitingPoint"));
    m_currentStatusMap.insertMulti("ZeroCorrection", BaseStatus("ZeroCorrection", "ZeroCorrection", "exception"));
    m_currentStatusMap.insertMulti("ZeroCorrection", BaseStatus("ZeroCorrection", "ZeroCorrection", "finish"));
    m_currentStatusMap.insertMulti("ZeroCorrection", BaseStatus("ZeroCorrection", "ZeroCorrection", "error"));

    //转等待点
    m_currentStatusMap.insertMulti("TurnToWaitingPoint", BaseStatus("TurnToWaitingPoint", "TurnToWaitingPoint", "TaskStart"));
    m_currentStatusMap.insertMulti("TurnToWaitingPoint", BaseStatus("TurnToWaitingPoint", "TurnToWaitingPoint", "exception"));
    m_currentStatusMap.insertMulti("TurnToWaitingPoint", BaseStatus("TurnToWaitingPoint", "TurnToWaitingPoint", "finish"));
    m_currentStatusMap.insertMulti("TurnToWaitingPoint", BaseStatus("TurnToWaitingPoint", "TurnToWaitingPoint", "error"));

    //任务开始
    m_currentStatusMap.insertMulti("TaskStart", BaseStatus("TaskStart", "TaskStart", "Calibration"));
    m_currentStatusMap.insertMulti("TaskStart", BaseStatus("TaskStart", "TaskStart", "exception"));
    m_currentStatusMap.insertMulti("TaskStart", BaseStatus("TaskStart", "TaskStart", "finish"));
    m_currentStatusMap.insertMulti("TaskStart", BaseStatus("TaskStart", "TaskStart", "error"));

    //校相处理
    m_currentStatusMap.insertMulti("Calibration", BaseStatus("Calibration", "Calibration", "CaptureTrackIng"));
    m_currentStatusMap.insertMulti("Calibration", BaseStatus("Calibration", "Calibration", "exception"));
    m_currentStatusMap.insertMulti("Calibration", BaseStatus("Calibration", "Calibration", "finish"));
    m_currentStatusMap.insertMulti("Calibration", BaseStatus("Calibration", "Calibration", "error"));

    //跟踪开始
    m_currentStatusMap.insertMulti("CaptureTrackIng", BaseStatus("CaptureTrackIng", "CaptureTrackIng", "GoUp"));
    m_currentStatusMap.insertMulti("CaptureTrackIng", BaseStatus("CaptureTrackIng", "CaptureTrackIng", "exception"));
    m_currentStatusMap.insertMulti("CaptureTrackIng", BaseStatus("CaptureTrackIng", "CaptureTrackIng", "finish"));
    m_currentStatusMap.insertMulti("CaptureTrackIng", BaseStatus("CaptureTrackIng", "CaptureTrackIng", "error"));

    //发上行
    m_currentStatusMap.insertMulti("GoUp", BaseStatus("GoUp", "GoUp", "TrackEnd"));
    m_currentStatusMap.insertMulti("GoUp", BaseStatus("GoUp", "GoUp", "exception"));
    m_currentStatusMap.insertMulti("GoUp", BaseStatus("GoUp", "GoUp", "finish"));
    m_currentStatusMap.insertMulti("GoUp", BaseStatus("GoUp", "GoUp", "error"));

    //跟踪结束
    m_currentStatusMap.insertMulti("TrackEnd", BaseStatus("TrackEnd", "TrackEnd", "PostProcessing"));
    m_currentStatusMap.insertMulti("TrackEnd", BaseStatus("TrackEnd", "TrackEnd", "exception"));
    m_currentStatusMap.insertMulti("TrackEnd", BaseStatus("TrackEnd", "TrackEnd", "finish"));
    m_currentStatusMap.insertMulti("TrackEnd", BaseStatus("TrackEnd", "TrackEnd", "error"));

    //事后处理
    m_currentStatusMap.insertMulti("PostProcessing", BaseStatus("PostProcessing", "PostProcessing", "exception"));
    m_currentStatusMap.insertMulti("PostProcessing", BaseStatus("PostProcessing", "PostProcessing", "finish"));
    m_currentStatusMap.insertMulti("PostProcessing", BaseStatus("PostProcessing", "PostProcessing", "error"));
}

void TaskMachine::acceptNotify(const QString& nextStatus)
{
    //等待前面一个线程完成
    if (m_workThread.isRunning())
    {
        m_workThread.quit();
        m_workThread.wait();
        delete m_currentEvent;
        m_currentEvent = nullptr;
    }

    if (nextStatus == "finish")  // 全部流程完成
    {
        // 推送当前完成的状态
        updateStatus(m_currentStatus, TaskStepStatus::Finish);
        TaskPlanSql::updataTaskStatus(mTaskPlanData.deviceWorkTask.m_uuid, TaskPlanStatus::Finish);
        mIsRunning = false;
    }
    else if (nextStatus == "error")  // 当前流程执行错误
    {
        //推送当前完成的状态
        updateStatus(m_currentStatus, TaskStepStatus::Error);
        TaskPlanSql::updataTaskStatus(mTaskPlanData.deviceWorkTask.m_uuid, TaskPlanStatus::Error);
        mIsRunning = false;
    }
    else if (nextStatus == "exception")  // 当前流程执行异常
    {
        //推送当前完成的状态
        updateStatus(m_currentStatus, TaskStepStatus::Exception);
        TaskPlanSql::updataTaskStatus(mTaskPlanData.deviceWorkTask.m_uuid, TaskPlanStatus::Exception);
        mIsRunning = false;
    }
    //    else  // 当前流程完成
    //    {
    //        // 推送当前完成的状态
    //        updateStatus(m_currentStatus, TaskStepStatus::Finish);
    //    }

    //状态切换，切换到下一个流程
    m_currentStatus = nextStatus;
}

BaseEvent* TaskMachine::instance(const QString& name)
{
    if (name == "Start")
    {
        return new Start();
    }
    if (name == "TaskPreparation")
    {
        return new TaskPreparation();
    }
    if (name == "ZeroCorrection")
    {
        return new ZeroCorrection();
    }
    if (name == "TaskStart")
    {
        return new TaskStart();
    }
    if (name == "TurnToWaitingPoint")
    {
        return new TurnToWaitingPoint();
    }
    if (name == "Calibration")
    {
        return new Calibration();
    }
    if (name == "CaptureTrackIng")
    {
        return new CaptureTrackIng();
    }
    if (name == "TrackEnd")
    {
        return new TrackEnd();
    }
    if (name == "PostProcessing")
    {
        return new PostProcessing();
    }
    if (name == "GoUp")
    {
        return new GoUp();
    }

    return nullptr;
}

QString TaskMachine::getMachineUUID() const { return m_machineUUID; }

void TaskMachine::setMachineUUID(const QString& machineUUID) { m_machineUUID = machineUUID; }
