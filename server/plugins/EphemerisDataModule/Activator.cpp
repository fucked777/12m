#include "CppMicroServicesUtility.h"
#include "EphemerisData.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{

    class US_ABI_LOCAL EphemerisDataModuleActivator : public BundleActivator
    {

    private:


    public:

        void Start(BundleContext context)
        {
            auto server = std::make_shared<EphemerisData>(context);
            ServiceProperties props;
            props["Type"] = std::string("EphemerisData");
            context.RegisterService<EphemerisData>(server, props);


            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("EphemerisData");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(EphemerisDataModuleActivator)
