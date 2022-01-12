#include "AutoTask4426Service.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL ParamMacroModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            //添加处理模块数据注册和响应请求
            auto clientRequest = std::make_shared<AutoTask4426Service>(context);
            clientRequest->setSrcAddr("AutoTask4426Service");
            ServiceProperties props;
            props["Type"] = std::string("AutoTask4426Service");
            context.RegisterService<AutoTask4426Service>(clientRequest, props);

            clientRequest->setConnectAddr(ZMQHelper::deaultLocalAddress());
            clientRequest->setSrcAddr("ServerAutoTaskService");
            QString msg;
            if (!clientRequest->startServer(&msg))
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(ParamMacroModuleActivator)
