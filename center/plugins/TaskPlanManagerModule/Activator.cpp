#include "CppMicroServicesUtility.h"
#include "ZMQUtility.h"

#include "ExtendedConfig.h"
#include "IGuiService.h"
#include "PlanListWidget.h"
#include "TaskPlanManagerService.h"

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
            explicit GuiImpl(BundleContext context)
                : mContext(context)
            {
            }
            virtual ~GuiImpl() {}

            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override
            {
                auto planListWidget = new PlanListWidget;
                if (mContext)
                {
                    auto ref = mContext.GetServiceReference<TaskPlanManagerService>();
                    auto service = mContext.GetService(ref);

                    QObject::connect(planListWidget, &PlanListWidget::sg_addItem, service.get(), &TaskPlanManagerService::addItem);
                    QObject::connect(planListWidget, &PlanListWidget::sg_deletePlanItem, service.get(), &TaskPlanManagerService::deletePlanItem);

                    QObject::connect(planListWidget, &PlanListWidget::sg_viewItem, service.get(), &TaskPlanManagerService::viewItem);
                    QObject::connect(planListWidget, &PlanListWidget::sg_editInfoItem, service.get(), &TaskPlanManagerService::editInfoItem);
                    QObject::connect(planListWidget, &PlanListWidget::sg_updateItem, service.get(), &TaskPlanManagerService::updateItem);
                    // QObject::connect(planListWidget, &PlanListWidget::sg_queryDeviceItem, service.get(), &TaskPlanManagerService::queryDeviceItem);
                    // QObject::connect(planListWidget, &PlanListWidget::sg_queryDTItem, service.get(), &TaskPlanManagerService::queryDTItem);

                    QObject::connect(service.get(), &TaskPlanManagerService::sg_addItemACK, planListWidget, &PlanListWidget::sg_addItemACK);
                    QObject::connect(service.get(), &TaskPlanManagerService::sg_deletePlanItemACK, planListWidget,
                                     &PlanListWidget::deletePlanItemACK);
                    QObject::connect(service.get(), &TaskPlanManagerService::sg_viewItemACK, planListWidget, &PlanListWidget::viewItemACK);
                    QObject::connect(service.get(), &TaskPlanManagerService::sg_editInfoItemACK, planListWidget, &PlanListWidget::editInfoItemACK);
                    QObject::connect(service.get(), &TaskPlanManagerService::sg_updateItemACK, planListWidget, &PlanListWidget::sg_updateItemACK);
                    // QObject::connect(service.get(), &TaskPlanManagerService::sg_queryDeviceItemACK, planListWidget,
                    //                  &PlanListWidget::queryDeviceItemACK);
                    // QObject::connect(service.get(), &TaskPlanManagerService::sg_queryDTItemACK, planListWidget, &PlanListWidget::queryDTItemACK);
                }
                return planListWidget;
            }

        private:
            BundleContext mContext;
        };

    public:
        void Start(BundleContext context)
        {
            auto taskPlanManagerWidgetGui = std::make_shared<GuiImpl>(context);
            ServiceProperties taskPlanManagerWidgetPops;
            taskPlanManagerWidgetPops["Type"] = std::string("TaskPlanManagerWidget");
            context.RegisterService<IGuiService>(taskPlanManagerWidgetGui, taskPlanManagerWidgetPops);

            registerNetService<TaskPlanManagerService>(context, "TaskPlanManagerService");
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TaskPlanManagerModuleActivator)
