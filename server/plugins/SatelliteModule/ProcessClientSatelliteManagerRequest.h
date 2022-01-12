#ifndef PROCESSCLIENTSatelliteSERVICE_H
#define PROCESSCLIENTSatelliteSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class ProcessClientSatelliteRequestImpl;
class ProcessClientSatelliteRequest : public INetMsg
{
public:
    ProcessClientSatelliteRequest(cppmicroservices::BundleContext context);
    ~ProcessClientSatelliteRequest();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void start();
    void stop();
    void AddSatellite(const ProtocolPack& pack);

    void EditSatellite(const ProtocolPack& pack);

    void DelSatellite(const ProtocolPack& pack);

    void GetSatellite(const ProtocolPack& pack);

    ProcessClientSatelliteRequestImpl* mImpl;
};
#endif  // PROCESSCLIENTSERVICE_H
