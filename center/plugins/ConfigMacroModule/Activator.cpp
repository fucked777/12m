#include "ConfigMacroManagerWidget.h"
#include "ConfigMacroService.h"
#include "ConfigMacroXMLReader.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
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
    class US_ABI_LOCAL ConfigMacroModuleActivator : public BundleActivator
    {
    private:
        class GuiImpl : public IGuiService
        {
        public:
            explicit GuiImpl(BundleContext context)
                : context_(context)
            {
            }
            virtual ~GuiImpl() {}

            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override
            {
                auto widget = new ConfigMacroManagerWidget;
                if (context_)
                {
                    auto ref = context_.GetServiceReference<ConfigMacroService>();
                    auto service = context_.GetService(ref);

                    QObject::connect(widget, &ConfigMacroManagerWidget::signalAddConfigMacro, service.get(), &ConfigMacroService::slotAddItem);
                    QObject::connect(widget, &ConfigMacroManagerWidget::signalEditConfigMacro, service.get(), &ConfigMacroService::slotModifyItem);
                    QObject::connect(widget, &ConfigMacroManagerWidget::signalDeleteConfigMacro, service.get(), &ConfigMacroService::slotDeleteItem);

                    QObject::connect(service.get(), &ConfigMacroService::signalAddItemACK, widget, &ConfigMacroManagerWidget::slotAddItemACK);
                    QObject::connect(service.get(), &ConfigMacroService::signalDeleteItemACK, widget, &ConfigMacroManagerWidget::slotDeleteItemACK);
                    QObject::connect(service.get(), &ConfigMacroService::signalModifyItemACK, widget, &ConfigMacroManagerWidget::slotModifyItemACK);
                }

                return widget;
            }

        private:
            BundleContext context_;
        };

    public:
        void Start(BundleContext context)
        {
            // 从配置文件中读取配置，保存到redis，用于生成配置宏界面和生成主界面手动控制界面
            ConfigMacroXMLReader reader;
            QList<ConfigMacroWorkMode> configMacroWorkModes;
            QString stationName;
            reader.getConfigMacroWorkMode(configMacroWorkModes, stationName);
            GlobalData::setConfigMacroWorkModeData(configMacroWorkModes);

            auto guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("ConfigMacroManagerWidget");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<ConfigMacroService>(context, "ConfigMacroManagerWidget");

            auto configMacroWidgetGui = std::make_shared<GuiImpl>(context);
            ServiceProperties configMacroWidgetPops;
            configMacroWidgetPops["Type"] = std::string("ConfigMacroManagerWidget");
            context.RegisterService<IGuiService>(configMacroWidgetGui, configMacroWidgetPops);
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(ConfigMacroModuleActivator)
