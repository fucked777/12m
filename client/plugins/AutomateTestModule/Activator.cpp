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

#include "AutomateTestModuleService.h"
#include "AutomateTestWidget.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "PlatformInterface.h"
#include "ZMQUtility.h"

/*
 * 20210525 wp??
 * 新版本这里的思路是
 * 界面配置文件读取过来
 * 每一个配置项都有一个key
 * 然后服务器通过key去执行相关的测试操作
 * 具体执行的方式和自动化运行一样参数配置文件读取
 * 流程写死
 */
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
    class US_ABI_LOCAL AutomateTestModuleActivator : public BundleActivator
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
                auto widget = new AutomateTestWidget(m_context);
                if (m_context)
                {
                    auto ref = m_context.GetServiceReference<AutomateTestModuleService>();
                    auto service = m_context.GetService(ref);
                    QObject::connect(widget, &AutomateTestWidget::sg_addTestItem, service.get(), &AutomateTestModuleService::addTestItem);
                    QObject::connect(widget, &AutomateTestWidget::sg_addTestBindItem, service.get(), &AutomateTestModuleService::addTestBindItem);
                    QObject::connect(widget, &AutomateTestWidget::sg_deleteTestBindItem, service.get(),
                                     &AutomateTestModuleService::deleteTestBindItem);
                    QObject::connect(widget, &AutomateTestWidget::sg_deleteTestItem, service.get(), &AutomateTestModuleService::deleteTestItem);
                    QObject::connect(widget, &AutomateTestWidget::sg_deleteTestResult, service.get(), &AutomateTestModuleService::deleteTestResult);
                    QObject::connect(widget, &AutomateTestWidget::sg_startTest, service.get(), &AutomateTestModuleService::startTest);
                    QObject::connect(widget, &AutomateTestWidget::sg_stopTest, service.get(), &AutomateTestModuleService::stopTest);
                    QObject::connect(widget, &AutomateTestWidget::sg_queryTestItem, service.get(), &AutomateTestModuleService::queryTestItem);
                    QObject::connect(widget, &AutomateTestWidget::sg_queryTestResult, service.get(), &AutomateTestModuleService::queryTestResult);
                    QObject::connect(widget, &AutomateTestWidget::sg_clearHistry, service.get(), &AutomateTestModuleService::clearHistry);

                    QObject::connect(service.get(), &AutomateTestModuleService::sg_addTestItemACK, widget, &AutomateTestWidget::sg_addTestItemACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_addTestBindItemACK, widget,
                                     &AutomateTestWidget::sg_addTestBindItemACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_deleteTestBindItemACK, widget,
                                     &AutomateTestWidget::sg_deleteTestBindItemACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_deleteTestItemACK, widget, &AutomateTestWidget::deleteTestItemACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_deleteTestResultACK, widget,
                                     &AutomateTestWidget::sg_deleteTestResultACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_startTestACK, widget, &AutomateTestWidget::startTestACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_stopTestACK, widget, &AutomateTestWidget::stopTestACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_queryTestItemACK, widget, &AutomateTestWidget::queryTestItemACK);
                    QObject::connect(service.get(), &AutomateTestModuleService::sg_queryTestResultACK, widget,
                                     &AutomateTestWidget::sg_queryTestResultACK);

                    QObject::connect(service.get(), &AutomateTestModuleService::sg_clearHistryACK, widget, &AutomateTestWidget::sg_clearHistryACK);
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

            registerNetService<AutomateTestModuleService>(context, "AutomateTestModuleService");
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

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(AutomateTestModuleActivator)
//![Activator]
