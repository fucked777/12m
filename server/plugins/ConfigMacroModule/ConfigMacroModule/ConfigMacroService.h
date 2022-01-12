#ifndef CONFIGMACROSERVICE_H
#define CONFIGMACROSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class ConfigMacroServiceImpl;
class ConfigMacroService : public INetMsg
{
public:
    ConfigMacroService(cppmicroservices::BundleContext context);
    ~ConfigMacroService();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void start();
    void stop();

    void addItem(const ProtocolPack& pack);
    void deleteItem(const ProtocolPack& pack);
    void queryItem(const ProtocolPack& pack);
    void modifyItem(const ProtocolPack& pack);

private:
    ConfigMacroServiceImpl* mImpl = nullptr;
};

#endif  // CONFIGMACROSERVICE_H
