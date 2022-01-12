#include "TaskScheduling.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QMetaType>

#include "CConverter.h"
#include "CaptureTrackIng.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskMachine.h"
#include "TaskPlanSql.h"
#include "HeartbeatMessageSerialize.h"

TaskScheduling::TaskScheduling(QObject* parent)
    : QThread(parent)
{
    TaskPlanSql::initDB();
}

void TaskScheduling::run()
{
    // 查询正在运行的计划，此时还没有任务机器，把所有状态为正在运行的计划设置为异常
    Optional<DeviceWorkTaskList> runningWorkTaskList = TaskPlanSql::queryRunningDevPlanMark();
    if (runningWorkTaskList.success())
    {
        for (auto deviceTask : runningWorkTaskList.value())
        {
            TaskPlanSql::updataTaskStatus(deviceTask.m_uuid, TaskPlanStatus::Exception);
        }
    }

    while (mIsRunning)
    {
        // 查询正在运行的计划
        Optional<DeviceWorkTaskList> runningWorkTaskList = TaskPlanSql::queryRunningDevPlanMark();
        if (runningWorkTaskList.success())
        {
            for (auto deviceTask : runningWorkTaskList.value())
            {
                // 如果该计划结束时间10秒后还没有结束，把计划状态设置为异常
                if (GlobalData::currentDateTime() > deviceTask.m_endTime.addSecs(10))
                {
                    TaskPlanSql::updataTaskStatus(deviceTask.m_uuid, TaskPlanStatus::Exception);
                }
            }
        }

        if (mCurrentTaskMachine)
        {
            // 更新当前任务的状态
            auto currentTask = TaskPlanSql::queryPlanByIDMark(mCurrentTaskMachine->deviceWorkTask().m_uuid);
            if (!currentTask.success())
            {
                mCurrentTaskMachine->stopRing();
                delete mCurrentTaskMachine;
                mCurrentTaskMachine = nullptr;
            }
            if (currentTask.success())
            {
                auto taskStatus = currentTask.value().deviceWorkTask.m_task_status;
                if (taskStatus == TaskPlanStatus::TimeOut || taskStatus == TaskPlanStatus::Finish || taskStatus == TaskPlanStatus::Error ||
                    taskStatus == TaskPlanStatus::Exception || taskStatus == TaskPlanStatus::CTNoStart)
                {
                    mCurrentTaskMachine->stopRing();
                    delete mCurrentTaskMachine;
                    mCurrentTaskMachine = nullptr;
                }
            }
        }

        DeviceWorkTask newDeviceTask;
        // 查询下一个未开始的新任务
        Optional<DeviceWorkTask> data = TaskPlanSql::queryNextNewDevPlan();
        if (data.success())
        {
            newDeviceTask = data.value();

            // 如果都没有目标 说明任务计划有问题
            if (newDeviceTask.m_totalTargetMap.isEmpty())
            {
                TaskPlanSql::updataTaskStatus(newDeviceTask.m_uuid, TaskPlanStatus::Error);
                continue;
            }

            // 如果都没有主跟目标 说明任务计划有问题
            DeviceWorkTaskTarget masterTargetItem;
            if (!newDeviceTask.getMainTarget(masterTargetItem))
            {
                TaskPlanSql::updataTaskStatus(newDeviceTask.m_uuid, TaskPlanStatus::Error);
                continue;
            }

            auto startDateTime = masterTargetItem.m_task_ready_start_time;
            QDateTime currentDateTime = GlobalData::currentDateTime();
            auto secs = currentDateTime.secsTo(startDateTime);
            // 当前时间已经超过开始时间
            if (secs < 0)
            {
                TaskPlanSql::updataTaskStatus(newDeviceTask.m_uuid, TaskPlanStatus::Error);
                continue;
            }
            // 是自动运行任务，提前60秒创建任务虚拟机
            if (mIsAutoRun && secs <= 60)
            {
                // 当前有任务，但是时间段还有冲突就把任务状态修改为冲突未执行，本地没有这种情况
                if (mCurrentTaskMachine)
                {
                    TaskPlanSql::updataTaskStatus(newDeviceTask.m_uuid, TaskPlanStatus::CTNoStart);
                }

                //启动任务计划
                TaskMachine* taskMachine = new TaskMachine;
                taskMachine->setDeviceWorkTask(newDeviceTask);
                connect(taskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);
                taskMachine->start();

                TaskPlanSql::updataTaskStatus(newDeviceTask.m_uuid, TaskPlanStatus::Running);

                mCurrentTaskMachine = taskMachine;
            }
        }

        // 如果当前有正在执行的任务，推送当前执行任务
        if (mCurrentTaskMachine)
        {
            mCurrentTaskPlan.deviceWorkTask = mCurrentTaskMachine->deviceWorkTask();
        }
        else
        {
            // 推送数据库查询出来的下一个将要执行的任务(可以没有下一个任务)
            mCurrentTaskPlan.deviceWorkTask = newDeviceTask;
        }

        // 推送任务计划信息
        publishTaskPlanInfo();

        msleep(900);
        QApplication::processEvents();  //让出当前线程  run线程和函数线程不在同一个线程里面，让出线程可以运行外部进来的信号。
    }
}

void TaskScheduling::stopRing()
{
    mIsRunning = false;
    quit();
}

void TaskScheduling::setIsAutoRun(bool isAutoRun)
{
    if (mIsAutoRun == isAutoRun)
    {
        return;
    }

    mIsAutoRun = isAutoRun;
    SystemLogPublish::infoMsg(mIsAutoRun ? "切换为自动运行任务" : "切换为手动运行任务");
}

void TaskScheduling::publishTaskPlanInfo()
{
    CurrentRunningTaskPlanData currentRunningTaskPlanData;
    // 初始化所有流程步骤都为未开始
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::START] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::TASK_PREPARATION] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::ZERO_CORRECTION] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::TASK_START] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::TURN_TOWAITINGPOINT] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::CALIBRATION] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::CAPTURE_TRACKLING] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::GO_UP] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::TRACK_END] = TaskStepStatus::NoStart;
    currentRunningTaskPlanData.taskStepStatusMap[TaskStep::POST_PROCESSING] = TaskStepStatus::NoStart;

    // 不为空表示此时有任务正在执行，推送正在执行的任务步骤
    if (mCurrentTaskMachine != nullptr)
    {
        currentRunningTaskPlanData.taskStepStatusMap = mCurrentTaskMachine->getTaskStepStatus();
    }

    currentRunningTaskPlanData.taskPlanData = mCurrentTaskPlan;

    AutoRunTaskStepStatusPublish::publish(currentRunningTaskPlanData);
}

void TaskScheduling::addParseDeviceTask(const QList<DeviceWorkTask>& deviceWorkTaskList) {}

void TaskScheduling::addParseDataTranWorkTask(const QList<DataTranWorkTask>& dataTranWorkTaskList) {}
