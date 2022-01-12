#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "ZMQLocalRouterServer.h"
#include "ZMQRouterServer.h"
#include "ZMQUtility.h"
#include <QDebug>
#include <memory>
#include <set>

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL ZMQServerActivator : public BundleActivator
    {
    public:
        void Start(BundleContext context)
        {
            QString msg;

            /* 本地router */
            auto localRouter = std::make_shared<ZMQLocalRouterServer>(context);
            ServiceProperties props;
            props["Type"] = std::string("ZMQLocalRouterServer");
            context.RegisterService<ZMQLocalRouterServer>(localRouter, props);

            auto communicationPort = ExtendedConfig::serverCommunicationPort();
            auto communicationIP = ExtendedConfig::serverCommunicationIP();
            auto serverAddr = ZMQHelper::syntheticAddress("tcp", communicationIP, communicationPort);
            auto connectRes = localRouter->startServer(serverAddr, &msg);
            if (!connectRes)
            {
                qWarning() << __FUNCTION__ << msg;
            }

            ////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////
            /* 远端router */
            auto router = std::make_shared<ZMQRouterServer>(context);
            props.clear();
            props["Type"] = std::string("ZMQRouterServer");
            context.RegisterService<ZMQRouterServer>(router, props);

            auto clientCommunicationPort = ExtendedConfig::clientCommunicationPort();
            auto serverCommunicationPort = ExtendedConfig::serverCommunicationPort();
            auto clientAddr = ZMQHelper::syntheticAddress("tcp", "*", clientCommunicationPort);
            serverAddr = ZMQHelper::syntheticAddress("tcp", "*", serverCommunicationPort);
            connectRes = router->startServer(serverAddr, clientAddr, &msg);

            if (!connectRes)
            {
                qWarning() << __FUNCTION__ << msg;
            }
        }

        void Stop(BundleContext /*context*/) {}

    private:
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(ZMQServerActivator)
