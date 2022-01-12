#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "PlatformInterface.h"
#include "SystemMainScreenModule/SystemMainScreen.h"
#include "SystemMainScreenModule/SystemMainScreenServer.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL SystemMainScreenModuleActivator : public BundleActivator
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
                auto systemMainScreen = new SystemMainScreen;
                if (mContext)
                {
                    auto ref = mContext.GetServiceReference<SystemMainScreenServer>();
                    auto service = mContext.GetService(ref);
                    QObject::connect(systemMainScreen, &SystemMainScreen::signalManualFunction, service.get(),
                                     &SystemMainScreenServer::slotManualFunction);
                    QObject::connect(systemMainScreen, &SystemMainScreen::signalsCmdDeviceJson, service.get(),
                                     &SystemMainScreenServer::slotSendProcessCmd);
                    QObject::connect(systemMainScreen, &SystemMainScreen::signalsUnitDeviceJson, service.get(),
                                     &SystemMainScreenServer::slotSendUnitParam);
                    QObject::connect(service.get(), &SystemMainScreenServer::signalsDeviceCMDResult, systemMainScreen,
                                     &SystemMainScreen::signalsDeviceCMDResult);
                }

                return systemMainScreen;
            }

        private:
            BundleContext mContext;
        };

    public:
        void Start(BundleContext context)
        {
            auto systemMainScreenGui = std::make_shared<GuiImpl>(context);
            ServiceProperties systemMainScreenGuiPops;
            systemMainScreenGuiPops["Type"] = std::string("SystemMainScreenWidget");
            context.RegisterService<IGuiService>(systemMainScreenGui, systemMainScreenGuiPops);

            // 添加与服务器通信的模块
            ServiceProperties props;
            auto server = std::make_shared<SystemMainScreenServer>(context);
            props["Type"] = std::string("SystemMainScreenServer");
            context.RegisterService<SystemMainScreenServer>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ClientSystemMainScreenServer");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(SystemMainScreenModuleActivator)
