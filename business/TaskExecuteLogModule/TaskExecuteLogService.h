#ifndef TaskExecuteLogSERVICE_H
#define TaskExecuteLogSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class TaskExecuteLogServiceImpl;
class TaskExecuteLogService : public INetMsg
{
    Q_OBJECT
public:
    explicit TaskExecuteLogService(cppmicroservices::BundleContext context);
    ~TaskExecuteLogService();

signals:
    void signalGetHistoryPlanACK(const QByteArray& data);
    void signalQueryAutoTaskLogACK(const QByteArray& data);

public slots:
    void slotGetHistoryPlan(const QByteArray& data);
    void slotQueryAutoTaskLog(const QByteArray& data);

private:
    TaskExecuteLogServiceImpl* m_impl;
};
#endif  // TaskExecuteLogSERVICE_H
