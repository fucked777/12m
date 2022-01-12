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
#include "PlatformInterface.h"
#include "%{HdrFileName}"
@if %{EnableServer}
#include "ExtendedConfig.h"
#include "%{ClientServiceHdr}"
#include "ZMQUtility.h"
#include <QDebug>
@endif

using namespace cppmicroservices;

@if %{EnableServer}
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
@endif


namespace
{

/**
 * This class implements a bundle activator that uses the bundle
 * context to register an English language dictionary service
 * with the C++ Micro Services registry during static initialization
 * of the bundle. The dictionary service interface is
 * defined in a separate file and is implemented by a nested class.
 */
class US_ABI_LOCAL %{ProjectName}Activator : public BundleActivator
{

private:
    class GuiImpl : public IGuiService
    {
    public:
        GuiImpl(BundleContext context) : m_context(context) {}
        virtual ~GuiImpl() {}

        virtual QWidget *createWidget(QWidget *parent = nullptr)
        {
            m_widget= new %{Class}(m_context, parent);
            return m_widget; 
        }
        QWidget* getWidget() override { return m_widget; }
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
        props["Type"] = std::string("%{Class}");
        context.RegisterService<IGuiService>(guiService, props);

@if %{EnableServer}
        registerNetService<%{ClientServiceName}>(context,"%{ClientServiceName}");
@endif

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
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(%{ProjectName}Activator)
//![Activator]
