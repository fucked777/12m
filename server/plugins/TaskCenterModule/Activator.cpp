#include "ExtendedConfig.h"
#include "TaskCenter.h"
#include "CppMicroServicesUtility.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL TaskCenterModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<TaskCenter>(context);
            ServiceProperties props;
            props["Type"] = std::string("TaskCenter");
            context.RegisterService<TaskCenter>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("TaskCenter");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TaskCenterModuleActivator)
