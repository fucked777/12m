#ifndef PARAMMACROHANDLER_H
#define PARAMMACROHANDLER_H

#include "PlanRunMessageDefine.h"
#include "SystemWorkMode.h"
#include <QObject>
#include <atomic>

class BaseParamMacroHandler;
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

private:
    void appendHandler(BaseParamMacroHandler* paramMacroHandler);

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
    QMap<SystemWorkMode, BaseParamMacroHandler*> mParamMacrohandleMap;
    // BaseParamMacroHandler* trkBBEHandler;
    /* 20210926 wp?? 退出的标志 */
    std::atomic<bool>* mRunningFlag;
};

#endif  // PARAMMACROHANDLER_H
