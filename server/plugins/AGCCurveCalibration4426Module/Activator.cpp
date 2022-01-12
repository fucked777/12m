#include "AGCManage.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL AutomateTestModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<AGCManage>(context);
            ServiceProperties props;
            props["Type"] = std::string("AGCCurveCalibration");
            context.RegisterService<AGCManage>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("AGCCurveCalibration");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(AutomateTestModuleActivator)
