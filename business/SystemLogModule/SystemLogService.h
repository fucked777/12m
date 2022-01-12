#ifndef SystemLogSERVICE_H
#define SystemLogSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

struct SystemLogData;
class SystemLogServiceImpl;
class SystemLogService : public INetMsg
{
    Q_OBJECT
public:
    explicit SystemLogService(cppmicroservices::BundleContext context);
    ~SystemLogService();

signals:
    void signalLogConditionQueryACK(const QByteArray& data);

public slots:
    void slotLogConditionQuery(const QByteArray& data);

private:
    SystemLogServiceImpl* mImpl;
};
#endif  // SystemLogSERVICE_H
