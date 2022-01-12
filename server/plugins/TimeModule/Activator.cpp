#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "IGuiService.h"
#include "ZMQUtility.h"

#include "GlobalData.h"
#include "TimeInfoXMLReader.h"
#include "TimeService.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL TimeModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<TimeService>(context);
            ServiceProperties props;
            props["Type"] = std::string("TimeService");
            context.RegisterService<TimeService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ServerTimeService");
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
