#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "IGuiService.h"
#include "TaskExecuteLogService.h"
#include "TaskExecuteLogWidget.h"
#include "TaskPlanManagerService.h"
#include "ZMQUtility.h"

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
    class US_ABI_LOCAL TaskPlanManagerModuleActivator : public BundleActivator
    {
    private:
        class GuiImpl : public IGuiService
        {
        public:
            GuiImpl(BundleContext context)
                : m_context(context)
            {
            }
            virtual ~GuiImpl() {}

            virtual QWidget* createWidget(const QString& /*subWidgetName*/)
            {
                TaskExecuteLogWidget* widget = new TaskExecuteLogWidget;
                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<TaskExecuteLogService>();
                    auto service = m_context.GetService(ref);
                    QObject::connect(widget, &TaskExecuteLogWidget::signalGetHistoryPlan, service.get(), &TaskExecuteLogService::slotGetHistoryPlan);
                    QObject::connect(service.get(), &TaskExecuteLogService::signalGetHistoryPlanACK, widget,
                                     &TaskExecuteLogWidget::slotGetHistoryPlan);
                    QObject::connect(widget, &TaskExecuteLogWidget::signalQueryAutoTaskLog, service.get(),
                                     &TaskExecuteLogService::slotQueryAutoTaskLog);
                    QObject::connect(service.get(), &TaskExecuteLogService::signalQueryAutoTaskLogACK, widget,
                                     &TaskExecuteLogWidget::slotQueryAutoTaskLog);
                }
                return widget;
            }

        private:
            BundleContext m_context;
        };

    public:
        void Start(BundleContext context)
        {
            auto server = std::make_shared<TaskPlanManagerService>(context);
            ServiceProperties props;
            props["Type"] = std::string("TaskPlanManagerService");
            context.RegisterService<TaskPlanManagerService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("TaskPlanManagerService");
            QString msg;
            if (!server->startServer(&msg))
            {
                qWarning() << __FUNCTION__ << msg;
            }

            std::shared_ptr<GuiImpl> guiService = std::make_shared<GuiImpl>(context);
            props.clear();
            props["Type"] = std::string("dialog");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<TaskExecuteLogService>(context, "ClientTaskExecuteLogService");
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TaskPlanManagerModuleActivator)
