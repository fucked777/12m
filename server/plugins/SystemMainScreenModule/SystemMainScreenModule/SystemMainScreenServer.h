#ifndef SYSTEMMAINSCREENSERVER_H
#define SYSTEMMAINSCREENSERVER_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

struct TimeConfigInfo;
class SystemMainScreenServerImpl;
class SystemMainScreenServer : public INetMsg
{
    Q_OBJECT
public:
    SystemMainScreenServer(cppmicroservices::BundleContext context);
    ~SystemMainScreenServer();

public slots:
    void slotTimeConfigChanged(const TimeConfigInfo& info);

private:
    SystemMainScreenServerImpl* mImpl;
};

#endif  // SYSTEMMAINSCREENSERVER_H
