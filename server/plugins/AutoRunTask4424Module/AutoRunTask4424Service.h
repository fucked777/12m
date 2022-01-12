#ifndef AUTORUNTASK4424SERVICE_H
#define AUTORUNTASK4424SERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

struct ManualMessage;
struct AutoTaskLogData;
class TaskScheduling;
class AutoRunTask4424Service : public INetMsg
{
public:
    AutoRunTask4424Service(cppmicroservices::BundleContext context);
    ~AutoRunTask4424Service();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void sendByteArrayToDevice(const QByteArray& data);

private slots:
    // 读取自动运行任务日志
    void slotReadAutoRunTaskLog(const AutoTaskLogData& data);

private:
    TaskScheduling* mTaskScheduling = nullptr;
};

#endif  // AUTORUNTASK4424SERVICE_H
