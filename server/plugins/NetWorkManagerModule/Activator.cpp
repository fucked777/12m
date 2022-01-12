/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

//! [Activator]

#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "NetStatusMonitor.h"
#include "NetWorkManager.h"
#include "PlatformInterface.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

template<typename T>
static void registerNetService(BundleContext context, const char* type, const char* addr)
{
    auto server = std::make_shared<T>(context);
    ServiceProperties props;
    props["Type"] = std::string(type);
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
    class US_ABI_LOCAL NetManagerActivator : public BundleActivator
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
                auto widget = new NetStatusMonitor;
                if (context_)
                {
                    auto ref = context_.GetServiceReference<NetWorkManager>();
                    auto service = context_.GetService(ref);
                    QObject::connect(service.get(), &NetWorkManager::sg_channelSendDataChange, widget, &NetStatusMonitor::channelSendDataChange);
                    QObject::connect(service.get(), &NetWorkManager::sg_channelRecvDataChange, widget, &NetStatusMonitor::channelRecvDataChange);
                }
                return widget;
            }

        private:
            BundleContext context_;
        };

    public:
        /**
         * Implements BundleActivator::Start(). Registers an
         * instance of a dictionary service using the bundle context;
         * attaches properties to the service that can be queried
         * when performing a service look-up.
         * @param context the context for the bundle.
         */
        void Start(BundleContext context)
        {
            auto systemMainScreenGui = std::make_shared<GuiImpl>(context);
            ServiceProperties systemMainScreenGuiPops;
            systemMainScreenGuiPops["Type"] = std::string("NetStatusMonitor");
            context.RegisterService<IGuiService>(systemMainScreenGui, systemMainScreenGuiPops);

            registerNetService<NetWorkManager>(context, "NetWorkManager", "NetWorkManager");
        }

        /**
         * Implements BundleActivator::Stop(). Does nothing since
         * the C++ Micro Services library will automatically unregister any registered services.
         * @param context the context for the bundle.
         */
        void Stop(BundleContext /*context*/) {}
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(NetManagerActivator)
