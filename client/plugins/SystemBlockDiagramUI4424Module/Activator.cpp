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
#include "PlatformInterface.h"
#include "SystemBlockDiagramService.h"
#include "SystemBlockDiagramWidget.h"
#include "ZMQUtility.h"
#include <QDebug>

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
    /**
     * This class implements a bundle activator that uses the bundle
     * context to register an English language dictionary service
     * with the C++ Micro Services registry during static initialization
     * of the bundle. The dictionary service interface is
     * defined in a separate file and is implemented by a nested class.
     */
    class US_ABI_LOCAL SystemBlockDiagramUIModuleActivator : public BundleActivator
    {
    private:
        class SystemBlockDiagramGuiImpl : public IGuiService
        {
        public:
            explicit SystemBlockDiagramGuiImpl(BundleContext context)
                : m_context(context)
            {
            }
            virtual ~SystemBlockDiagramGuiImpl() {}

            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override
            {
                auto tempWidget = new SystemBlockDiagramWidget(m_context);
                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<SystemBlockDiagramService>();
                    auto service = m_context.GetService(ref);
                    Q_ASSERT(service != nullptr);
                    QObject::connect(tempWidget, &SystemBlockDiagramWidget::sg_sendProcessCmd, service.get(),
                                     &SystemBlockDiagramService::sendProcessDeviceJson);
                    QObject::connect(tempWidget, &SystemBlockDiagramWidget::sg_sendUnitCmd, service.get(),
                                     &SystemBlockDiagramService::sendUnitDeviceJson);
                }

                return tempWidget;
            }

        private:
            BundleContext m_context;
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
            auto guiService = std::make_shared<SystemBlockDiagramGuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("SystemBlockDiagramWidget");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<SystemBlockDiagramService>(context, "SystemBlockDiagramService");
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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(SystemBlockDiagramUIModuleActivator)
//![Activator]
