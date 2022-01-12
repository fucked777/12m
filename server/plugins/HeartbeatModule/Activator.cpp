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
#include "HaSwitchWidget.h"
#include "IGuiService.h"
#include "HeartbeatThread.h"
#include <QDebug>

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL HeartBeatModuleActivator : public BundleActivator
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

            virtual QWidget* createWidget(const QString& /*subWidgetName*/) override { return new HaSwitchWidget(context_); }

        private:
            BundleContext context_;
        };

    public:
        void Start(BundleContext context)
        {
            auto ha_switchGUI = std::make_shared<GuiImpl>(context);
            ServiceProperties ha_switchGUIPops;
            ha_switchGUIPops["Type"] = std::string("dialog");
            context.RegisterService<IGuiService>(ha_switchGUI, ha_switchGUIPops);

            auto server = std::make_shared<HeartbeatThread>(context);
            ServiceProperties props;
            props["Type"] = std::string("HeartbeatThread");
            context.RegisterService<HeartbeatThread>(server, props);
            server->startHeartbeat();
            /* 延时等待线程启动 */
            QThread::sleep(3);
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(HeartBeatModuleActivator)
