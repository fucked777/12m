#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

#include "ConfigMacroService.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL ConfigMacroModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            //添加处理模块数据注册和响应请求
            auto server = std::make_shared<ConfigMacroService>(context);
            server->setSrcAddr("ConfigMacroService");
            ServiceProperties props;
            props["Type"] = std::string("ConfigMacroService");
            context.RegisterService<ConfigMacroService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ServerConfigMacroService");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(ConfigMacroModuleActivator)
