#include "TaskScheduling.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QMetaType>

#include "CConverter.h"
#include "CaptureTrackIng.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PlanRunMessageSerialize.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskMachine.h"
#include "TaskPlanSql.h"

TaskScheduling::TaskScheduling(QObject* parent)
    : QThread(parent)
{
    TaskPlanSql::initDB();
}
void TaskScheduling::restoreTaskRuning()
{
    start();
}

void TaskScheduling::restoreTask()
{
    auto clearTaskFunc=[](){
        // 查询正在运行的计划，此时还没有任务机器，把所有状态为正在运行的计划设置为异常
        Optional<DeviceWorkTaskList> runningWorkTaskList = TaskPlanSql::queryRunningDevPlanMark();
        if (runningWorkTaskList.success())
        {
            for (auto& deviceTask : runningWorkTaskList.value())
            {
                TaskPlanSql::updataTaskStatus(deviceTask.m_uuid, TaskPlanStatus::Exception);
            }
        }
    };

    if(mCurrentTaskMachine != nullptr)
    {
        mCurrentTaskMachine->stopRing();
        delete mCurrentTaskMachine;
        mCurrentTaskMachine = nullptr;
    }

    /* 获取未完成的任务信息 */
    auto runningTask = AutoRunTaskStepStatusPublish::getRunningTask();
    AutoRunTaskStepStatusPublish::clearRunningTask();
    auto curDate = GlobalData::currentDateTime();

    /* 没有计划信息或者计划已经过期 */
    if(runningTask.taskPlanData.deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        clearTaskFunc();
        return;
    }
    if(runningTask.taskPlanData.deviceWorkTask.m_endTime <= curDate)
    {
        SystemLogPublish::infoMsg(QString("任务已过期放弃恢复:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        clearTaskFunc();
        return;
    }
    /* 未开始的任务 */
    if(runningTask.taskPlanData.deviceWorkTask.m_task_status == TaskPlanStatus::NoStart)
    {
        clearTaskFunc();
        return;
    }
    /* 任务数据还原 */
    auto taskTimeListResult = TaskRunCommonHelper::getDeviceWorkTaskInfo(runningTask.taskPlanData);
    if (!taskTimeListResult)
    {
        clearTaskFunc();
        SystemLogPublish::infoMsg(QString("任务数据恢复错误:%1").arg(taskTimeListResult.msg()));
        return;
    }
    auto taskTimeList = taskTimeListResult.value();
    taskTimeList.manualMessage = runningTask.msg;
    if(taskTimeList.manualMessage.masterTaskCode != taskTimeList.masterSataTaskCode)
    {
        clearTaskFunc();
        SystemLogPublish::infoMsg(QString("任务数据恢复错误:解析任务代号和当前任务代号不一致%1--%2").arg(taskTimeList.manualMessage.masterTaskCode, taskTimeList.masterSataTaskCode));
        return;
    }
    auto machineUUID = QUuid::createUuid().toString();
    TaskGlobalInfoSingleton::getInstance().setTaskTimeList(machineUUID, taskTimeList);


    /* 步骤如果已经到任务结束那就没必要继续了 */
    if(runningTask.taskStepStatusMap.value(TaskStep::TRACK_END) != TaskStepStatus::NoStart)
    {
        SystemLogPublish::infoMsg(QString("任务即将结束放弃恢复:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        auto msg = runningTask.msg;
        msg.manualType = ManualType::ResourceRelease;
        emit sg_taskResourceRelease(msg);

        clearTaskFunc();
        return;
    }
    auto step = TaskStep::TRACK_END;
    auto status = runningTask.taskStepStatusMap.value(step);
    if(status != TaskStepStatus::NoStart)
    {
        SystemLogPublish::infoMsg(QString("任务恢复到等待跟踪结束:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);
        mCurrentTaskMachine->restoreTaskRuning(step, machineUUID, runningTask.taskStepStatusMap);

        return;
    }

    step = TaskStep::GO_UP;
    status = runningTask.taskStepStatusMap.value(step);
    if(status == TaskStepStatus::Continue || //
       status == TaskStepStatus::Exception || //
       status == TaskStepStatus::Error     ||//
       status == TaskStepStatus::Finish      )
    {
        SystemLogPublish::infoMsg(QString("任务恢复到等待跟踪结束:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);
        mCurrentTaskMachine->restoreTaskRuning(TaskStep::TRACK_END, machineUUID, runningTask.taskStepStatusMap);
        return;
    }
    else if(status == TaskStepStatus::Running ||//
            status == TaskStepStatus::Start)
    {
        SystemLogPublish::infoMsg(QString("任务从发上行恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(step, machineUUID, runningTask.taskStepStatusMap);
        return;
    }


    step = TaskStep::CAPTURE_TRACKLING;
    status = runningTask.taskStepStatusMap.value(step);
    if(status == TaskStepStatus::Running ||//
       status == TaskStepStatus::Start)
    {
        SystemLogPublish::infoMsg(QString("任务从捕获跟踪恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(step, machineUUID, runningTask.taskStepStatusMap);
        return;
    }
    else if(status == TaskStepStatus::Continue || //
       status == TaskStepStatus::Exception || //
       status == TaskStepStatus::Error     ||//
       status == TaskStepStatus::Finish      )
    {
        SystemLogPublish::infoMsg(QString("任务从发上行恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(TaskStep::GO_UP, machineUUID, runningTask.taskStepStatusMap);
        return;
    }


    step = TaskStep::CALIBRATION;
    status = runningTask.taskStepStatusMap.value(step);
    if(status == TaskStepStatus::Running ||//
       status == TaskStepStatus::Start)
    {
        SystemLogPublish::infoMsg(QString("任务从校相恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(step, machineUUID, runningTask.taskStepStatusMap);
        return;
    }
    else if(status == TaskStepStatus::Continue || //
       status == TaskStepStatus::Exception || //
       status == TaskStepStatus::Error     ||//
       status == TaskStepStatus::Finish      )
    {
        SystemLogPublish::infoMsg(QString("任务从捕获跟踪恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(TaskStep::CAPTURE_TRACKLING, machineUUID, runningTask.taskStepStatusMap);
        return;
    }

    step = TaskStep::TURN_TOWAITINGPOINT;
    status = runningTask.taskStepStatusMap.value(step);
    if(status == TaskStepStatus::Running ||//
       status == TaskStepStatus::Start)
    {
        SystemLogPublish::infoMsg(QString("任务从转等待点恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(step, machineUUID, runningTask.taskStepStatusMap);
        return;
    }
    else if(status == TaskStepStatus::Continue || //
       status == TaskStepStatus::Exception || //
       status == TaskStepStatus::Error     ||//
       status == TaskStepStatus::Finish      )
    {
        SystemLogPublish::infoMsg(QString("任务从校相恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(TaskStep::TASK_START, machineUUID, runningTask.taskStepStatusMap);
        return;
    }

    step = TaskStep::ZERO_CORRECTION;
    status = runningTask.taskStepStatusMap.value(step);
    if(status == TaskStepStatus::Running ||//
       status == TaskStepStatus::Start)
    {
        SystemLogPublish::infoMsg(QString("任务从转等待点恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(step, machineUUID, runningTask.taskStepStatusMap);
        return;
    }
    else if(status == TaskStepStatus::Continue || //
       status == TaskStepStatus::Exception || //
       status == TaskStepStatus::Error     ||//
       status == TaskStepStatus::Finish      )
    {
        SystemLogPublish::infoMsg(QString("任务从校相恢复中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
        mCurrentTaskMachine = new TaskMachine;
        mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
        connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

        mCurrentTaskMachine->restoreTaskRuning(TaskStep::TURN_TOWAITINGPOINT, machineUUID, runningTask.taskStepStatusMap);
        return;
    }

    SystemLogPublish::infoMsg(QString("任务重新开始中:%1").arg(runningTask.taskPlanData.deviceWorkTask.m_plan_serial_number));
    mCurrentTaskMachine = new TaskMachine;
    mCurrentTaskMachine->setTaskPlan(runningTask.taskPlanData);
    connect(mCurrentTaskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);

    mCurrentTaskMachine->restoreTaskRuning(TaskStep::START, machineUUID, runningTask.taskStepStatusMap);
}

void TaskScheduling::run()
{
    qWarning() << "任务调度线程启动中";
    mIsRunning = true;

    QThread::sleep(5);
    restoreTask();

    while (mIsRunning)
    {
        // 查询正在运行的计划
        Optional<DeviceWorkTaskList> runningWorkTaskList = TaskPlanSql::queryRunningDevPlanMark();
        if (runningWorkTaskList.success())
        {
            for (auto& deviceTask : runningWorkTaskList.value())
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
            auto currentTask = TaskPlanSql::queryPlanByIDMark(mCurrentTaskMachine->curTaskDevUuid());
            /* 这个是查不到了 就是被删除了或者数据库查询错误 */
            if (!currentTask.success())
            {
                /* 这里删除任务的处理是直接向手动控制发送资源释放指令 */
                auto manualMessage = TaskGlobalInfoSingleton::getInstance().taskTimeList().manualMessage;
                manualMessage.manualType = ManualType::ResourceRelease;
                emit sg_taskResourceRelease(manualMessage);


                mCurrentTaskMachine->stopRing();
                delete mCurrentTaskMachine;
                mCurrentTaskMachine = nullptr;

                mCurrentTaskPlan = TaskPlanData();
                GlobalData::cleanTaskRunningInfo();
            }
            /* 这个是查到了 但是状态不正或者说已经退出了 */
            if (currentTask.success())
            {
                auto taskStatus = currentTask.value().deviceWorkTask.m_task_status;
                if (taskStatus == TaskPlanStatus::TimeOut || taskStatus == TaskPlanStatus::Finish || taskStatus == TaskPlanStatus::Error ||
                    taskStatus == TaskPlanStatus::Exception || taskStatus == TaskPlanStatus::CTNoStart)
                {
                    if (taskStatus == TaskPlanStatus::Finish)
                    {
                        /* 这里删除任务的处理是直接向手动控制发送资源释放指令 */
                        auto manualMessage = TaskGlobalInfoSingleton::getInstance().taskTimeList().manualMessage;
                        manualMessage.manualType = ManualType::ResourceRelease;
                        emit sg_taskResourceRelease(manualMessage);
                    }

                    mCurrentTaskMachine->stopRing();
                    delete mCurrentTaskMachine;
                    mCurrentTaskMachine = nullptr;

                    mCurrentTaskPlan = TaskPlanData();
                    GlobalData::cleanTaskRunningInfo();
                }
            }
        }

        TaskPlanData taskPlanData;
        // 查询下一个未开始的新任务
        auto data = TaskPlanSql::queryNextNewPlan();
        if (data.success())
        {
            taskPlanData = data.value();
            auto& deviceWorkTask = taskPlanData.deviceWorkTask;

            // 如果都没有目标 说明任务计划有问题
            if (deviceWorkTask.m_totalTargetMap.isEmpty())
            {
                TaskPlanSql::updataTaskStatus(deviceWorkTask.m_uuid, TaskPlanStatus::Error);
                continue;
            }

            // 如果都没有主跟目标 说明任务计划有问题
            DeviceWorkTaskTarget masterTargetItem;
            if (!deviceWorkTask.getMainTarget(masterTargetItem))
            {
                TaskPlanSql::updataTaskStatus(deviceWorkTask.m_uuid, TaskPlanStatus::Error);
                continue;
            }

            auto startDateTime = masterTargetItem.m_task_ready_start_time;
            QDateTime currentDateTime = GlobalData::currentDateTime();
            auto secs = currentDateTime.secsTo(startDateTime);
            // 当前时间已经超过开始时间
            if (secs < 0)
            {
                TaskPlanSql::updataTaskStatus(deviceWorkTask.m_uuid, TaskPlanStatus::Error);
                continue;
            }
            // 是自动运行任务，提前3秒创建任务虚拟机
            if (secs <= 3)
            {
                /* 不处于自动化运行 */
                if (!GlobalData::getAutoRunTaskFlag())
                {
                    SystemLogPublish::infoMsg("当前处于手动运行停止,跳过任务运行");
                    TaskPlanSql::updataTaskStatus(deviceWorkTask.m_uuid, TaskPlanStatus::Error);
                    continue;
                }

                // 当前有任务，但是时间段还有冲突就把任务状态修改为冲突未执行，本地没有这种情况
                if (mCurrentTaskMachine)
                {
                    TaskPlanSql::updataTaskStatus(deviceWorkTask.m_uuid, TaskPlanStatus::CTNoStart);
                    continue;
                }

                //启动任务计划
                auto taskMachine = new TaskMachine;
                taskMachine->setTaskPlan(taskPlanData);
                connect(taskMachine, &TaskMachine::signalSendToDevice, this, &TaskScheduling::signalSendToDevice);
                taskMachine->start();

                TaskPlanSql::updataTaskStatus(deviceWorkTask.m_uuid, TaskPlanStatus::Running);
                mCurrentTaskMachine = taskMachine;
            }
        }

        if(mCurrentTaskMachine != nullptr)
        {
            mCurrentTaskPlan = mCurrentTaskMachine->taskPlan();
        }
        else
        {
            mCurrentTaskPlan = taskPlanData;
        }
        // 推送任务计划信息
        publishTaskPlanInfo();

        for (auto i = 0; i < 10; i++)
        {
            QThread::msleep(100);
            QApplication::processEvents();  //让出当前线程  run线程和函数线程不在同一个线程里面，让出线程可以运行外部进来的信号。
        }
    }

    qWarning() << "任务调度线程退出";
}

void TaskScheduling::stopRing()
{
    qWarning() << "任务调度退出中";
    mIsRunning = false;
    quit();
    while (isRunning() && !isFinished())
    {
        QThread::msleep(10);
        QApplication::processEvents();
    }
    if(mCurrentTaskMachine!=nullptr)
    {
        mCurrentTaskMachine->stopRing();
        delete mCurrentTaskMachine;
        mCurrentTaskMachine = nullptr;
    }
    wait();

    qWarning() << "任务调度退出完成";
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
        mCurrentTaskPlan.deviceWorkTask.m_task_status = TaskPlanStatus::Running;
    }

    currentRunningTaskPlanData.taskPlanData = mCurrentTaskPlan;

    AutoRunTaskStepStatusPublish::publish(currentRunningTaskPlanData);
}

void TaskScheduling::addParseDeviceTask(const QList<DeviceWorkTask>& deviceWorkTaskList)
{
    Optional<DeviceWorkTaskList> nostartWorkTaskList = Optional<DeviceWorkTaskList>::emptyOptional();
    //如果是占用就删除非占用的未执行的任务计划
    if (mCurrentTaskPlan.deviceWorkTask.m_working_mode == "ZY")
    {
        //查询出未执行的任务计划 ，删除未执行的任务计划
        nostartWorkTaskList = TaskPlanSql::queryNoStartDevPlan();
    }
    else
    {
        //查询出未执行和正在运行的任务计划 ，删除未执行的任务计划
        nostartWorkTaskList = TaskPlanSql::queryRunningOrNoStartDevPlan();
    }
    int delSuccess = 0;
    for (auto& item : nostartWorkTaskList.value())
    {
        auto ans = TaskPlanSql::deletePlan(item.m_uuid);
        if (ans.success())
        {
            delSuccess++;
        }
    }
    if (nostartWorkTaskList.value().size() == delSuccess)
    {
        //计划全部正常删除
    }
    else
    {
        //有计划未正常删除
    }
    //插入新的任务计划
    int insertSuccess = 0;
    for (auto& item : deviceWorkTaskList)
    {
        auto ans = TaskPlanSql::insertPlan(item);
        if (ans.success())
        {
            insertSuccess++;
        }else{
            SystemLogPublish::errorMsg(ans.msg());
        }
    }
}

void TaskScheduling::addParseDataTranWorkTask(const QList<DataTranWorkTask>& /*dataTranWorkTaskList*/) {}
