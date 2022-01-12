#ifndef TASKSCHEDULING_H
#define TASKSCHEDULING_H

#include "AutoRunTaskLogMessageDefine.h"
#include "RedisHelper.h"
#include "TaskPlanMessageDefine.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QQueue>
#include <QThread>

class TaskMachine;
//这里继承一个线程 实现run方法来轮训计划列表和读取数据
class TaskScheduling : public QThread
{
    Q_OBJECT
public:
    explicit TaskScheduling(QObject* parent = nullptr);
    void run() override;

public:
    void stopRing();
    // 设置是否进行自动运行任务
    void setIsAutoRun(bool isAutoRun);

private:
    // 推送当前正在运行或者下一个将要运行的任务计划信息
    void publishTaskPlanInfo();

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);
    //推送实时计划到客户端
    void signalSendRealTimePlanData2Client(uint src, uint pdxpType, uint msgType, const QByteArray& msg);

public slots:
    //添加解析完成的设备工作计划
    void addParseDeviceTask(const QList<DeviceWorkTask>& deviceWorkTaskList);
    //添加解析完成的设备工作计划
    void addParseDataTranWorkTask(const QList<DataTranWorkTask>& dataTranWorkTaskList);

private:
    bool mIsRunning = true;  // 是否运行

    TaskMachine* mCurrentTaskMachine = nullptr;  // 执行任务的机器
    TaskPlanData mCurrentTaskPlan;               // 当前设备工作计划

    bool mIsAutoRun = true;  // 是否自动运行任务
    bool mIsMaster = false;
};

#endif  // TASKSCHEDULING_H
