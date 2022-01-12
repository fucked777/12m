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
#include "IGuiService.h"
#include "SystemMainScreen.h"
#include "SystemMainScreenServer.h"
#include "ZMQUtility.h"

using namespace cppmicroservices;

namespace
{
    /**
     * This class implements a bundle activator that uses the bundle
     * context to register an English language dictionary service
     * with the C++ Micro Services registry during static initialization
     * of the bundle. The dictionary service interface is
     * defined in a separate file and is implemented by a nested class.
     */
    class US_ABI_LOCAL SystemMainScreenModuleActivator : public BundleActivator
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

            QWidget* createWidget(const QString&) override
            {
                auto systemMainScreen = new SystemMainScreen;

                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<SystemMainScreenServer>();
                    auto server = m_context.GetService(ref);
                    QObject::connect(systemMainScreen, &SystemMainScreen::signalTimeConfigChanged, server.get(),
                                     &SystemMainScreenServer::slotTimeConfigChanged);
                }

                m_widget = systemMainScreen;
                return m_widget;
            }

        private:
            BundleContext m_context;
            QWidget* m_widget{ nullptr };
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
            auto guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("SystemMainScreen");
            context.RegisterService<IGuiService>(guiService, props);

            auto server = std::make_shared<SystemMainScreenServer>(context);
            ServiceProperties props2;
            props2["Type"] = std::string("SystemMainScreenServer");
            context.RegisterService<SystemMainScreenServer>(server, props2);

            auto communicationPort = ExtendedConfig::serverCommunicationPort();
            auto communicationIP = ExtendedConfig::serverCommunicationIP();
            auto connectAddr = ZMQHelper::syntheticAddress("tcp", communicationIP, communicationPort);
            server->setConnectAddr(connectAddr);
            server->setSrcAddr("ServerSystemMainScreenServer");
            QString msg;
            if (!server->startServer(&msg))
            {
                qWarning() << __FUNCTION__ << msg;
            }
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(SystemMainScreenModuleActivator)
//![Activator]
