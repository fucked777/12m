#ifndef CONTROLFLOWHANDLER_H
#define CONTROLFLOWHANDLER_H

#include "PlanRunMessageDefine.h"
#include "Utility.h"

#include <QObject>
#include <QScopedPointer>
#include <TaskPlanMessageDefine.h>
#include <atomic>

class ParamMacroDispatcher;
class ConfigMacroDispatcher;
class ResourceReleaseDispatcher;
class OnekeyXLHandler;
class OnekeyXXHandler;
class ControlFlowHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlFlowHandler)
public:
    ControlFlowHandler(QObject* parent = nullptr);
    ~ControlFlowHandler();

    /*
     * 当check 为true masterTaskCode参数无效
     */
    void handle(const ManualMessage& msg, bool isCheck = true);

    void setPlanInfo(const TaskPlanData& taskPlanData);
    void setRunningFlag(std::atomic<bool>* runningFlag);
    bool isRunning();
    bool isExit();

    ManualMessage getNewManualMsg() const;

    ConfigMacroData getNewConfigMacroData() const;

    /*
     *
     * 20211021 wp 新增
     * 原因是再没有加这个之前只能先下宏然后才能得到处理过的宏数据
     * 所以就是先下宏才能检查模式和设备 不太对头所以增加一个函数来单独干这个事儿
     */
    static OptionalNotValue allocConfigMacro(ManualMessage& msg, bool isCheck = true);

    // 检查和修正任务结构
    static OptionalNotValue checkAndFillManualMessage(ManualMessage& msg);

private:
    // 获取设备组合号
    void getDevNumber();

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
    QScopedPointer<ParamMacroDispatcher> mParamMacroDispatcher;
    QScopedPointer<ConfigMacroDispatcher> mConfigMacroDispatcher;
    QScopedPointer<ResourceReleaseDispatcher> mResourceReleaseDispatcher;
    QScopedPointer<OnekeyXLHandler> mOnekeyXLHandler;
    QScopedPointer<OnekeyXXHandler> mOnekeyXXHandler;
    ManualMessage mManualMessage;
    TaskPlanData mTaskPlanData; /* 当前计划信息没有就为空 */
    /* 退出标志 */
    std::atomic<bool>* mRunningFlag{ nullptr };
};

#endif  // CONTROLFLOWHANDLER_H
