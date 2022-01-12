#ifndef PARAMMACROSERVICE_H
#define PARAMMACROSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class ParamMacroServiceImpl;
class ParamMacroService : public INetMsg
{
public:
    ParamMacroService(cppmicroservices::BundleContext context);
    ~ParamMacroService();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void start();
    void stop();

    void addItem(const ProtocolPack& pack);
    void modifyItem(const ProtocolPack& pack);
    void deleteItem(const ProtocolPack& pack);
    void queryItem(const ProtocolPack& pack);

private:
    ParamMacroServiceImpl* mImpl;
};

#endif  // PARAMMACROSERVICE_H
