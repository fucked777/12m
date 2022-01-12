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
#include "SatelliteService.h"
#include "SatelliteUi.h"
#include "ZMQUtility.h"
#include <memory>
#include <set>

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
    class US_ABI_LOCAL SatelliteActivator : public BundleActivator
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
                auto satelliteWidget = new SatelliteUi(context_);
                if (context_)
                {
                    auto ref = context_.GetServiceReference<SatelliteService>();
                    auto service = context_.GetService(ref);

                    /* 添加卫星 */
                    QObject::connect(satelliteWidget, &SatelliteUi::signalAddSatellite, service.get(), &SatelliteService::slotAddSatellite);
                    /* 编辑卫星 */
                    QObject::connect(satelliteWidget, &SatelliteUi::signalEditSatellite, service.get(), &SatelliteService::slotEditSatellite);

                    /* 删除卫星 */
                    QObject::connect(satelliteWidget, &SatelliteUi::signalDelSatellite, service.get(), &SatelliteService::slotDelSatellite);

                    /* 获取所有的卫星接口 */
                    QObject::connect(satelliteWidget, &SatelliteUi::signalGetAllSatellite, service.get(), &SatelliteService::slotGetAllSatellite);

                    QObject::connect(service.get(), &SatelliteService::signalAddSatelliteResponce, satelliteWidget,
                                     &SatelliteUi::signalAddSatelliteResponce);

                    QObject::connect(service.get(), &SatelliteService::signalEditSatelliteResponce, satelliteWidget,
                                     &SatelliteUi::signalEditSatelliteResponce);

                    QObject::connect(service.get(), &SatelliteService::signalDelSatelliteResponce, satelliteWidget,
                                     &SatelliteUi::slotDelSatelliteResponce);

                    QObject::connect(service.get(), &SatelliteService::signalAllSatelliteResponce, satelliteWidget,
                                     &SatelliteUi::slotAllSatelliteResponce);
                }

                return satelliteWidget;
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
            {
                auto server = std::make_shared<SatelliteService>(context);
                ServiceProperties props;
                props["Type"] = std::string("SatelliteService");
                context.RegisterService<SatelliteService>(server, props);

                server->setConnectAddr(ZMQHelper::deaultLocalAddress());
                server->setSrcAddr("SatelliteService");
                QString msg;
                if (!server->startServer(&msg))
                {
                    qWarning() << __FUNCTION__ << msg;
                }
            }

            std::shared_ptr<GuiImpl> guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("dialog");
            context.RegisterService<IGuiService>(guiService, props);
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

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(SatelliteActivator)
//![Activator]
