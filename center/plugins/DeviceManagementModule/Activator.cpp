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
#include "DMService.h"
#include "DMWidget.h"
#include "ExtendedConfig.h"
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
    /**
     * This class implements a bundle activator that uses the bundle
     * context to register an English language dictionary service
     * with the C++ Micro Services registry during static initialization
     * of the bundle. The dictionary service interface is
     * defined in a separate file and is implemented by a nested class.
     */
    class US_ABI_LOCAL Activator : public BundleActivator
    {
    private:
        class GuiImpl : public IGuiService
        {
        public:
            explicit GuiImpl(BundleContext context)
                : m_context(context)
            {
            }
            virtual ~GuiImpl() {}

            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override
            {
                auto dmWidget = new DMWidget(m_context);
                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<DMService>();
                    auto service = m_context.GetService(ref);
                    QObject::connect(dmWidget, &DMWidget::sg_add, service.get(), &DMService::addItem);
                    QObject::connect(dmWidget, &DMWidget::sg_delete, service.get(), &DMService::deleteItem);
                    // QObject::connect(widget, &DMWidget::sg_query, service.get(), &DMService::queryItem);
                    QObject::connect(dmWidget, &DMWidget::sg_modify, service.get(), &DMService::modifyItem);

                    QObject::connect(service.get(), &DMService::sg_addACK, dmWidget, &DMWidget::sg_addACK);
                    QObject::connect(service.get(), &DMService::sg_deleteACK, dmWidget, &DMWidget::deleteACK);
                    // QObject::connect(service.get(), &DMService::sg_queryACK, widget, &DMWidget::queryACK);
                    QObject::connect(service.get(), &DMService::sg_modifyACK, dmWidget, &DMWidget::sg_modifyACK);
                }

                return dmWidget;
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
            auto guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("DMWidget");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<DMService>(context, "DMService");
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

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(Activator)
//![Activator]
