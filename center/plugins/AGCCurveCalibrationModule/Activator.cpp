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

#include "AGCCurveCalibration.h"
#include "AGCCurveCalibrationService.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "PlatformInterface.h"
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
    class US_ABI_LOCAL AGCCurveCalibrationModuleActivator : public BundleActivator
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
                auto widget = new AGCCurveCalibration(m_context);
                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<AGCCurveCalibrationService>();
                    auto service = m_context.GetService(ref);
                    QObject::connect(widget, &AGCCurveCalibration::sg_selectAGCData, service.get(), &AGCCurveCalibrationService::selectAGCData);
                    QObject::connect(widget, &AGCCurveCalibration::sg_saveAGCData, service.get(), &AGCCurveCalibrationService::saveAGCData);
                    QObject::connect(widget, &AGCCurveCalibration::sg_startAGCData, service.get(), &AGCCurveCalibrationService::startAGCData);
                    QObject::connect(widget, &AGCCurveCalibration::sg_endAGCData, service.get(), &AGCCurveCalibrationService::endAGCData);
                    QObject::connect(widget, &AGCCurveCalibration::sg_deleteAGCData, service.get(), &AGCCurveCalibrationService::deleteAGCData);
                    QObject::connect(widget, &AGCCurveCalibration::sg_finishingItem, service.get(), &AGCCurveCalibrationService::finishingItem);
                    QObject::connect(widget, &AGCCurveCalibration::sg_sendToMACB, service.get(), &AGCCurveCalibrationService::sendToMACB);
                    QObject::connect(widget, &AGCCurveCalibration::sg_sendToACU, service.get(), &AGCCurveCalibrationService::sendToACU);

                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_selectAGCDataACK, widget, &AGCCurveCalibration::selectAGCDataACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_saveAGCDataACK, widget, &AGCCurveCalibration::saveAGCDataACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_startAGCDataACK, widget, &AGCCurveCalibration::startAGCDataACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_endAGCDataACK, widget, &AGCCurveCalibration::endAGCDataACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_deleteAGCDataACK, widget, &AGCCurveCalibration::deleteAGCDataACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_finishingItemACK, widget, &AGCCurveCalibration::finishingItemACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_sendToMACBACK, widget, &AGCCurveCalibration::sendToMACBACK);
                    QObject::connect(service.get(), &AGCCurveCalibrationService::sg_sendToACUACK, widget, &AGCCurveCalibration::sendToACUACK);
                }

                return widget;
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
            std::shared_ptr<GuiImpl> guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("dialog");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<AGCCurveCalibrationService>(context, "AGCCurveCalibrationService");
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

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(AGCCurveCalibrationModuleActivator)
//![Activator]
