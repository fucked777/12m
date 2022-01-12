#ifndef ResourceRestructuringSERVICE_H
#define ResourceRestructuringSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class ResourceRestructuringServiceImpl;
class ResourceRestructuringService : public INetMsg
{
public:
    ResourceRestructuringService(cppmicroservices::BundleContext context);
    ~ResourceRestructuringService();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void start();
    void stop();
    void addItem(const ProtocolPack& pack);
    void deleteItem(const ProtocolPack& pack);
    void queryItem(const ProtocolPack& pack);
    void modifyItem(const ProtocolPack& pack);

private:
    ResourceRestructuringServiceImpl* mImpl;
};

#endif  // ResourceRestructuringSERVICE_H
