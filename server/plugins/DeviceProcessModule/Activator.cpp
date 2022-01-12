#include "CommunicationDefine.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessService.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

template<typename T>
static void registerNetService(BundleContext context, const QString& type, const QString& addr)
{
    /* 20211104 wp?在修改之前接收数据是没问题的
     * 解析数据可能会出现冲突或者解析不过来的情况
     * 这里把线程名字接口放出来还是写的都一样
     * 如果解析不过来再换名字
     */
    auto server = std::make_shared<T>("DeviceProcessServiceThread", context);
    ServiceProperties props;
    props["Type"] = type.toStdString();
    context.RegisterService<T>(server, props);

    server->setConnectAddr(ZMQHelper::deaultLocalAddress());
    server->setSrcAddr(addr);
    QString msg;
    if (!server->startServer(&msg))
    {
        qWarning() << __FUNCTION__ << msg;
    }
}

namespace
{
    class US_ABI_LOCAL DeviceProcessModuleActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            registerNetService<DeviceProcessService>(context,"DeviceProcessService" , "ServerDeviceProcessService");
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(DeviceProcessModuleActivator)
