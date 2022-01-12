#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ResourceRestructuringService.h"
#include "ZMQHelper.h"
using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL ResourceRestructuringModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<ResourceRestructuringService>(context);
            ServiceProperties props;
            props["Type"] = std::string("ResourceRestructuringService");
            context.RegisterService<ResourceRestructuringService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ServerResourceRestructuringService");
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
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(ResourceRestructuringModuleActivator)
