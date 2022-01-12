
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

/* 2021021 wp?? 套一层类防止重名 消除未使用警告 */
class BaseEventHelper
{
public:
    // 日期转为积日，默认从2000年1月1日开始，记为1
    static qint64 convertToAccumDate(const QDate& date, const QDate& startDate = QDate(2000, 1, 1)) { return startDate.daysTo(date) + 1; }

    // 时间转为BCD时间
    static int converToBCDTime(const QTime& time) { return time.msecsSinceStartOfDay(); }
    // 时间转为BCD日期
    static int converToBCDDate(const QDateTime& time) { return time.toString("yyyyMMdd").toInt(nullptr, 16); }
    static int converToDayTime(const QDateTime& time) { return QTime::fromString(time.toString("hhmmsszzz"), "hhmmsszzz").msecsSinceStartOfDay(); }
};

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
    virtual void notify(const QString& nextStatus);  //状态改变

    void notifyInfoLog(const QString& logStr);     //提示日志
    void notifyWarningLog(const QString& logStr);  //警告日志
    void notifyErrorLog(const QString& logStr);    //错误日志

    // 执行命令，并且等待命令响应
    bool waitExecSuccess(const PackCommand& packCommand, uint ttl = 5);

    //获取站资源数据
    static bool getCurrentStationUACAddr(qint32& ckAddr, QString& errMsg);
    /* 获取当前在线(主机)ACU */
    static Optional<DeviceID> getOnlineACU();
    //获取DTE设备在线状态，哪台在线用哪台
    bool dteOnlineCheck();

    void setIsExit(bool isExit);

signals:
    void start();
    void notifyMachine(const QString& nextStatus);  //通知事件状态已经改变，比如说异常，或者正确执行，或者超时等

    void signalExitRun(bool isExit);

    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);

public slots:
    void startEvent();  //这里指的是具体事务需要执行的操作

public:
    TaskMachine* taskMachine = nullptr;

protected:
    PackCommand m_packCommand;
    SingleCommandHelper m_singleCommandHelper;
    std::atomic<bool> m_isExit{ false };
    TaskPlanData m_taskPlanData;
    DeviceWorkTask deviceWorkTask;
};

#endif  // BASEEVENT_H
