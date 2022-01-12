#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"
#include "microservice.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL DeviceStatusLogModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            //            auto server = std::make_shared<MicroService>(context);
            //            ServiceProperties props;
            //            props["Type"] = std::string("MicroService");
            //            context.RegisterService<MicroService>(server, props);

            //            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            //            server->setSrcAddr("MicroService");
            //            QString msg;
            //            if (!server->startServer(&msg))
            //            {
            //                qWarning() << __FUNCTION__ << msg;
            //            }
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(DeviceStatusLogModuleActivator)
