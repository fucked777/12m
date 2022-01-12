#include "CppMicroServicesUtility.h"
#include "DataTransmissionCenter.h"
#include "ExtendedConfig.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{

    class US_ABI_LOCAL DataTransmissionCenterModuleActivator : public BundleActivator
    {

    private:


    public:

        void Start(BundleContext context)
        {
            auto server = std::make_shared<DataTransmissionCenter>(context);
            ServiceProperties props;
            props["Type"] = std::string("DataTransmissionCenter");
            context.RegisterService<DataTransmissionCenter>(server, props);


            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("DataTransmissionCenter");
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
}

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(DataTransmissionCenterModuleActivator)
