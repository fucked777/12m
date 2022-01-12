#include "CppMicroServicesUtility.h"
#include "%{HdrFileName}"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{

class US_ABI_LOCAL %{ProjectName}Activator : public BundleActivator
{

private:


public:

    void Start(BundleContext context)
    {
        auto server = std::make_shared<%{CN}>(context);
        ServiceProperties props;
        props["Type"] = std::string("%{CN}");
        context.RegisterService<%{CN}>(server, props);


        server->setConnectAddr(ZMQHelper::deaultLocalAddress());
        server->setSrcAddr("%{CN}");
        QString msg;
        if (!server->startServer(&msg))
        {
            qWarning() << __FUNCTION__ << msg;
        }
    }


    void Stop(BundleContext /*context*/)
    {
        // NOTE: The service is automatically unregistered
    }
};
} 

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(%{ProjectName}Activator)
