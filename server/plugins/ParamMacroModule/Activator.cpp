#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ParamMacroService.h"
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
            auto clientRequest = std::make_shared<ParamMacroService>(context);
            clientRequest->setSrcAddr("ParamMacroService");
            ServiceProperties props;
            props["Type"] = std::string("ParamMacroService");
            context.RegisterService<ParamMacroService>(clientRequest, props);

            clientRequest->setConnectAddr(ZMQHelper::deaultLocalAddress());
            clientRequest->setSrcAddr("ServerParamMacroService");
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
