#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "IGuiService.h"
#include "StatusPersistenceService.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL StatusPersistenceModuleActivator : public BundleActivator
    {
    private:
//        class GuiImpl : public IGuiService
//        {
//        public:
//            GuiImpl(BundleContext context)
//                : m_context(context)
//            {
//            }
//            virtual ~GuiImpl() {}

//            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override
//            {
//                auto widget = new DeviceStatusLogWidget;
//                return widget;
//            }

//        private:
//            BundleContext m_context;
//        };

    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<StatusPersistenceService>(context);
            ServiceProperties props;
            props["Type"] = std::string("StatusPersistenceService");
            context.RegisterService<StatusPersistenceService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ServerStatusPersistenceService");
            QString msg;
            if (!server->startServer(&msg))
            {
                qWarning() << __FUNCTION__ << msg;
            }

//            auto guiService = std::make_shared<GuiImpl>(context);
//            props.clear();
//            props["Type"] = std::string("DeviceStatusLogWidget");
//            context.RegisterService<IGuiService>(guiService, props);
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(StatusPersistenceModuleActivator)
