#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "IGuiService.h"
#include "ZMQUtility.h"

#include "FepManagerShell.h"
#include "GlobalData.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL TimeModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<FepManagerShell>(context);
            ServiceProperties props;
            props["Type"] = std::string("FepManagerShell");
            context.RegisterService<FepManagerShell>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("FepService");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TimeModuleActivator)
