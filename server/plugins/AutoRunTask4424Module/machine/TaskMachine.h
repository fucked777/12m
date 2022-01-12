
#ifndef TASKMACHINE_H
#define TASKMACHINE_H

#include "BaseStatus.h"
#include "TaskPlanMessageDefine.h"
#include "TaskRunCommonHelper.h"
#include <QDateTime>
#include <QMap>
#include <QMultiMap>
#include <QObject>
#include <QQueue>
#include <QThread>

class BaseEvent;
struct AutoTaskLog;
//抽象一个任务计划作为一个线程，这里同样继承一个线程中

class TaskMachine : public QThread
{
    Q_OBJECT
public:
    explicit TaskMachine(QObject* parent = nullptr);
    ~TaskMachine();

    QString currentStatus();  //当前状态
    void init();              //初始化状态机器
    void stopRing();
    DeviceWorkTask deviceWorkTask() const;
    void setDeviceWorkTask(const DeviceWorkTask& deviceWorkTask);

    QList<DataTranWorkTask> getDataTranWorkTaskList() const;
    void setDataTranWorkTaskList(const QList<DataTranWorkTask>& dataTranWorkTaskList);

    virtual void run();  //每一个任务计划执行的对应线程

    void notifyInfoLog(const QString& logStr);     //普通日志
    void notifyWarningLog(const QString& logStr);  //警告日志
    void notifyErrorLog(const QString& logStr);    //错误日志

    void updateStatus(const QString& event, TaskStepStatus status);

    void stopPlanRunning(TaskTimeList& taskTimelist);

    // 获取任务每个步骤的状态
    QMap<TaskStep, TaskStepStatus> getTaskStepStatus() const;

signals:
    void signalSendToDevice(const QByteArray& data);  //发送数据到设备

public slots:
    void acceptNotify(const QString& nextStatus);  //接收事件通知

public:
    BaseEvent* instance(const QString& name);

    DeviceWorkTaskTarget m_mainTaskTarget;  //主跟目标信息
    QString getMachineUUID() const;
    void setMachineUUID(const QString& machineUUID);

private:
    QString m_currentStatus = "start";                  // 当前流程的状态
    QString mCurrentRunningStatus = "";                 // 当前运行的状态
    bool mIsRunning = true;                             // 标识线程是否运行
    QDateTime m_taskStartTime;                          // 任务开始时间
    QDateTime m_taskEndTime;                            // 任务结束时间
    QMultiMap<QString, BaseStatus> m_currentStatusMap;  // 当前机器的状态集合  必须包含 开始和结束   //这里是约束条件
    QThread* m_workThread = nullptr;                    // 动态工作者线程
    DeviceWorkTask mDeviceWorkTask;
    QList<DataTranWorkTask> mDataTranWorkTaskList;  // 设备计划对应的数传计划
    QString m_machineUUID;
    BaseEvent* currentEvent = nullptr;

    QMap<TaskStep, TaskStepStatus> mStepStatusMap;  // 储存当前运行的任务每一个步骤的状态
};

#endif  // TASKMACHINE_H
