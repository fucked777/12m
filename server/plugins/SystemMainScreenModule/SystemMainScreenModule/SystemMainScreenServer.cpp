#include "SystemMainScreenServer.h"

#include "CppMicroServicesUtility.h"
#include "TimeMessageSerialize.h"

class SystemMainScreenServerImpl
{
    DIS_COPY_MOVE(SystemMainScreenServerImpl)
public:
    inline SystemMainScreenServerImpl() {}
    inline ~SystemMainScreenServerImpl() {}
    cppmicroservices::BundleContext context;
};

SystemMainScreenServer::SystemMainScreenServer(cppmicroservices::BundleContext context)
    : INetMsg()
    , mImpl(new SystemMainScreenServerImpl)
{
    mImpl->context = context;
}

SystemMainScreenServer::~SystemMainScreenServer() { delete mImpl; }

void SystemMainScreenServer::slotTimeConfigChanged(const TimeConfigInfo& info)
{
    QString json;
    json << info;

    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerTimeService";
    pack.module = true;
    pack.operation = "SlotSaveTimeConfigInfo";
    pack.data = json.toUtf8();
    silenceSend(pack);
}
