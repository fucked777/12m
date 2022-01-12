#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "IGuiService.h"
#include "ZMQUtility.h"

#include "SystemLogService.h"
#include "SystemLogSubscriber.h"
#include "SystemLogWidget.h"

using namespace cppmicroservices;

template<typename T>
static void registerNetService(BundleContext context, const char* type)
{
    auto server = std::make_shared<T>(context);
    ServiceProperties props;
    props["Type"] = std::string(type);
    context.RegisterService<T>(server, props);

    server->setConnectAddr(ZMQHelper::deaultLocalAddress());
    server->setSrcAddr(type);
    QString msg;
    if (!server->startServer(&msg))
    {
        qWarning() << __FUNCTION__ << msg;
    }
}

namespace
{
    class US_ABI_LOCAL SystemLogModuleActivator : public BundleActivator
    {
    private:
        class GuiImpl : public IGuiService
        {
        public:
            GuiImpl(BundleContext context)
                : context_(context)
            {
            }
            virtual ~GuiImpl() {}

            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override
            {
                SystemLogWidget* widget = new SystemLogWidget;
                if (context_)
                {
                    auto ref = context_.GetServiceReference<SystemLogService>();
                    auto service = context_.GetService(ref);
                    QObject::connect(widget, &SystemLogWidget::signalLogConditionQuery, service.get(), &SystemLogService::slotLogConditionQuery);
                    QObject::connect(service.get(), &SystemLogService::signalLogConditionQueryACK, widget, &SystemLogWidget::slotLogConditionQuery);
                }
                return widget;
            }

        private:
            BundleContext context_;
        };

    public:
        void Start(BundleContext context)
        {
            std::shared_ptr<GuiImpl> guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("dialog");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<SystemLogService>(context, "ServerSystemLogService");

            new SystemLogSubscriber;
        }

        /**
         * Implements BundleActivator::Stop(). Does nothing since
         * the C++ Micro Services library will automatically unregister any registered services.
         * @param context the context for the bundle.
         */
        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(SystemLogModuleActivator)
