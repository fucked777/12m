#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "TaskReportService.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL TaskReportTableModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<TaskReportService>(context);
            ServiceProperties props;
            props["Type"] = std::string("TaskReportService");
            context.RegisterService<TaskReportService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ServerTaskReportTableService");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TaskReportTableModuleActivator)
