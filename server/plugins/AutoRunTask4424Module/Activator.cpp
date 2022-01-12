#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

#include "AutoRunTask4424Service.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL AutoRun4424ModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            //添加处理模块数据注册和响应请求
            auto autoRunTask4424Service = std::make_shared<AutoRunTask4424Service>(context);
            autoRunTask4424Service->setSrcAddr("AutoRunTask4424Service");
            ServiceProperties props;
            props["Type"] = std::string("AutoRunTask4424Service");
            context.RegisterService<AutoRunTask4424Service>(autoRunTask4424Service, props);

            autoRunTask4424Service->setConnectAddr(ZMQHelper::deaultLocalAddress());
            autoRunTask4424Service->setSrcAddr("ServerAutoRunTask4424Service");
            QString msg;
            if (!autoRunTask4424Service->startServer(&msg))
            {
                qWarning() << __FUNCTION__ << msg;
            }
        }

        void Stop(BundleContext /*context*/) {}
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(AutoRun4424ModuleActivator)
