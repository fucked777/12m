#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

#include "AutoRunTask4426Service.h"

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
            auto autoRunTask4426Service = std::make_shared<AutoRunTask4426Service>(context);
            autoRunTask4426Service->setSrcAddr("AutoRunTask4426Service");
            ServiceProperties props;
            props["Type"] = std::string("AutoRunTask4426Service");
            context.RegisterService<AutoRunTask4426Service>(autoRunTask4426Service, props);

            autoRunTask4426Service->setConnectAddr(ZMQHelper::deaultLocalAddress());
            autoRunTask4426Service->setSrcAddr("ServerAutoRunTask4426Service");
            QString msg;
            if (!autoRunTask4426Service->startServer(&msg))
            {
                qWarning() << __FUNCTION__ << msg;
            }
        }

        void Stop(BundleContext /*context*/) {}
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(AutoRun4424ModuleActivator)
