#ifndef RESOURCERELEASEDISPATCHER_H
#define RESOURCERELEASEDISPATCHER_H

#include "PlanRunMessageDefine.h"
#include "SystemWorkMode.h"
#include <QObject>
#include <atomic>

class BaseHandler;
class BaseResourceReleaseHandler;
class ResourceReleaseDispatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ResourceReleaseDispatcher)
public:
    ResourceReleaseDispatcher();
    ~ResourceReleaseDispatcher();
    void handle(const ManualMessage& msg);
    void setRunningFlag(std::atomic<bool>* runningFlag);
    bool isRunning();
    bool isExit();

private:
    void appendHandler(BaseResourceReleaseHandler* resourceReleaseHandler);

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
    QMap<SystemWorkMode, BaseResourceReleaseHandler*> mResourceReleaseHandlerMap;
    BaseHandler* mRestLink{ nullptr };
    std::atomic<bool>* mRunningFlag{ nullptr };
};

#endif  // RESOURCERELEASEDISPATCHER_H
