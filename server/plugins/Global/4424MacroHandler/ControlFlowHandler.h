#ifndef CONTROLFLOWHANDLER_H
#define CONTROLFLOWHANDLER_H

#include "PlanRunMessageDefine.h"

#include <QObject>
#include <QScopedPointer>
#include <atomic>

#include "Utility.h"

class ParamMacroDispatcher;
class ConfigMacroDispatcher;
class ResourceReleaseDispatcher;
class OnekeyXLHandler;
class ControlFlowHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlFlowHandler)
public:
    ControlFlowHandler(QObject* parent = nullptr);
    ~ControlFlowHandler();

    void handle(const ManualMessage& msg, const QString& masterTaskCode = QString());
    void exit();

    ManualMessage getNewManualMsg() const;
    ConfigMacroData getNewConfigMacroData() const;
    static OptionalNotValue allocConfigMacro(ManualMessage& msg, const QString& masterTaskCode = QString());

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);

    // 提示信息
    void signalInfoMsg(const QString& msg);
    // 警告信息
    void signalWarningMsg(const QString& msg);
    // 错误信息
    void signalErrorMsg(const QString& msg);

private:
    QScopedPointer<ParamMacroDispatcher> mParamMacroDispatcher;
    QScopedPointer<ConfigMacroDispatcher> mConfigMacroDispatcher;
    QScopedPointer<ResourceReleaseDispatcher> mResourceReleaseDispatcher;
    QScopedPointer<OnekeyXLHandler> mOnekeyXLHandler;
    ManualMessage mManualMessage;
    /* 退出标志 */
    std::atomic<bool> mRunningFlag;
};

#endif  // CONTROLFLOWHANDLER_H
