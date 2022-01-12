#ifndef TaskReportTableSERVICE_H
#define TaskReportTableSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class TaskReportTableServiceImpl;
class TaskReportTableService : public INetMsg
{
    Q_OBJECT
public:
    TaskReportTableService(cppmicroservices::BundleContext context);
    ~TaskReportTableService();

private:
    void getHistoryPlanACK(const ProtocolPack& pack);
    void getDataByUUIDACK(const ProtocolPack& pack);

signals:
    void signalGetHistoryPlanACK(const QByteArray& data);
    void signalGetDataByUUIDACK(const QByteArray& data);

public slots:
    void slotGetHistoryPlan(const QByteArray& data);
    void slotGetDataByUUID(const QByteArray& data);

private:
    TaskReportTableServiceImpl* m_impl;
};
#endif  // TaskReportTableSERVICE_H
