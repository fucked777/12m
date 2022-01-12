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

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"

#include "ControlCmdResponseUnpacker.h"
#include "ControlResultReportUnpacker.h"
#include "ExtensionStatusReportUnpacker.h"
#include "PacketHandleService.h"
#include "ProcessControlCmdPacker.h"
#include "StatusQueryCmdPacker.h"
#include "UnitParamSetPacker.h"

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
    class US_ABI_LOCAL PacketHandleActivator : public BundleActivator
    {
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
            std::shared_ptr<PacketHandleService> service = std::make_shared<PacketHandleService>();
            ServiceProperties props;
            props["Type"] = std::string("PacketHandleService");
            context.RegisterService<IPacketHandleService>(service, props);

            service->appendPacker(new UnitParamSetPacker());
            service->appendPacker(new ProcessControlCmdPacker());
            service->appendPacker(new StatusQueryCmdPacker());

            service->appendUnpacker(new ControlCmdResponseUnpacker());
            service->appendUnpacker(new ControlResultReportUnpacker());
            service->appendUnpacker(new ExtensionStatusReportUnpacker());
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

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(PacketHandleActivator)
//![Activator]
