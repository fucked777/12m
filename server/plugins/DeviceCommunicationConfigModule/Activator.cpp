#include "DevProtocolSerialize.h"
#include "DeviceCommunicationConfig.h"
#include "ExtendedConfig.h"
#include "CppMicroServicesUtility.h"
#include <QByteArray>
#include "ZMQUtility.h"

/*
 * 20210227 wp??
 *
 * Net接口            OK
 * qt property接口    OK
 * redis 接口         不提供
 */
using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL DeviceCommunicationConfigActivator : public BundleActivator
    {
    private:
    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<DeviceCommunicationConfig>(context);
            ServiceProperties props;
            props["Type"] = std::string("DeviceCommunicationConfig");
            context.RegisterService<DeviceCommunicationConfig>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("DeviceCommunicationConfig");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(DeviceCommunicationConfigActivator)
