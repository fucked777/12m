
#ifndef BASEEVENT_H
#define BASEEVENT_H

#include "BusinessMacroCommon.h"
#include "INetMsg.h"
#include "ProtocolXmlTypeDefine.h"
#include "SingleCommandHelper.h"
#include "TaskMachine.h"
#include <QMetaType>
#include <QMutex>
#include <QObject>
#include <atomic>

struct AutoTaskLog;
class BaseEvent : public QObject
{
    Q_OBJECT
public:
    BaseEvent(QObject* parent = nullptr);
    virtual ~BaseEvent();
    virtual void doSomething();                    //这里指的是具体事务需要执行的操作
    virtual void dispatch(const QByteArray& arr);  //这里泛指数据改变，或者从IO获取数据信息
    virtual void registerMachine(TaskMachine* taskMachine);

    void notify(const QString& nextStatus);  //状态改变
    QString nextStatus();

    void notifyInfoLog(const QString& logStr);     //提示日志
    void notifyWarningLog(const QString& logStr);  //警告日志
    void notifyErrorLog(const QString& logStr);    //错误日志
    void notifySpecificTipsLog(const QString& logStr);

    // 执行命令，并且等待命令响应
    bool waitExecSuccess(const PackCommand& packCommand, uint ttl = 10);

    void setRunningFlag(std::atomic<bool>* runningFlag);
    bool isRunning();
    bool isExit();

signals:
    void start();
    //void notifyMachine(const QString& nextStatus);  //通知事件状态已经改变，比如说异常，或者正确执行，或者超时等
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);

public slots:
    void startEvent();  //这里指的是具体事务需要执行的操作

public:
    TaskMachine* taskMachine = nullptr;

protected:
    PackCommand m_packCommand;
    SingleCommandHelper m_singleCommandHelper;
    std::atomic<bool>* m_runningFlag{ nullptr };
    TaskPlanData m_taskPlanData;
    QString m_nextStatus;
    std::atomic<bool> m_statusChangeMutex;
};

#endif  // BASEEVENT_H
