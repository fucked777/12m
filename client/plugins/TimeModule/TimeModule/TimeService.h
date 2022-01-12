#ifndef TIMESERVICE_H
#define TIMESERVICE_H

#include <QObject>

namespace cppmicroservices
{
    class BundleContext;
}

struct TimeData;
class TimeServiceImpl;
class TimeService : public QObject
{
    Q_OBJECT
public:
    TimeService(cppmicroservices::BundleContext context);
    ~TimeService();

signals:
    void sg_start();

private:
    TimeServiceImpl* mImpl;
};
#endif  // TIMESERVICE_H
