#ifndef PARAMMACROHANDLER_H
#define PARAMMACROHANDLER_H

#include "PlanRunMessageDefine.h"
#include "SystemWorkMode.h"
#include <QObject>
#include <QScopedPointer>
#include <atomic>

struct TaskPlanData;
class BaseParamMacroHandler;
class ParamMacroDispatcherImpl;
class ParamMacroDispatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ParamMacroDispatcher)
public:
    explicit ParamMacroDispatcher(QObject* parent = nullptr);
    ~ParamMacroDispatcher();

    void handle(const ManualMessage& msg);

    void setRunningFlag(std::atomic<bool>* runningFlag);
    bool isRunning();
    bool isExit();
    void setCCZFPlan(const TaskPlanData& taskPlanData);

private:
    void appendHandler(BaseParamMacroHandler* paramMacroHandler);
    /* 极化/旋向检查 */
    void polarizationCheck(const ManualMessage& msg);

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);
    // 提示信息
    void signalInfoMsg(const QString& msg);
    // 警告信息
    void signalWarningMsg(const QString& msg);
    // 错误信息
    void signalErrorMsg(const QString& msg);
    //
    void signalSpecificTipsMsg(const QString& msg);

private:
    QScopedPointer<ParamMacroDispatcherImpl> m_impl;
};

#endif  // PARAMMACROHANDLER_H
