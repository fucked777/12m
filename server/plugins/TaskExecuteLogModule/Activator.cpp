#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "TaskExecuteLogService.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL TaskExecuteLogModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<TaskExecuteLogService>(context);
            ServiceProperties props;
            props["Type"] = std::string("TaskExecuteLogModule");
            context.RegisterService<TaskExecuteLogService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ServerTaskExecuteLogService");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TaskExecuteLogModuleActivator)
