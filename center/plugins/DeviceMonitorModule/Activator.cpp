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

#include "DeviceProcessService.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "ServiceHelper.h"

#include "CppMicroServicesUtility.h"
#include "CreateMonitorWidget.h"
#include "PlatformInterface.h"
#include "ZMQUtility.h"
#include <QMessageBox>
#include <QObject>
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
    class US_ABI_LOCAL DeviceMonitorModuleActivator : public BundleActivator
    {
    private:
        class SystemMonitorGuiImpl : public IGuiService
        {
        public:
            explicit SystemMonitorGuiImpl(BundleContext context)
                : m_context(context)
            {
            }
            virtual ~SystemMonitorGuiImpl() {}

            virtual QWidget* createWidget(const QString& subWidgetName) override
            {
                std::shared_ptr<DeviceProcessService> service;
                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<DeviceProcessService>();
                    service = m_context.GetService(ref);
                }

                return CreateMonitorWidget::createWidget(subWidgetName, service);
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
            std::shared_ptr<SystemMonitorGuiImpl> guiService = std::make_shared<SystemMonitorGuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("SystemMonitorWidget");
            context.RegisterService<IGuiService>(guiService, props);

            //添加与服务器通信的模块
            auto server = std::make_shared<DeviceProcessService>(context);
            props["Type"] = std::string("DeviceProcessService");
            context.RegisterService<DeviceProcessService>(server, props);

            server->setConnectAddr(ZMQHelper::deaultLocalAddress());
            server->setSrcAddr("ClientDeviceProcessService");
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

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(DeviceMonitorModuleActivator)
//![Activator]
