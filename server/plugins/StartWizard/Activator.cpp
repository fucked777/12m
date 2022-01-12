#include "CppMicroServicesUtility.h"
#include "IGuiService.h"
#include "StartWizard.h"

using namespace cppmicroservices;

namespace
{
    class US_ABI_LOCAL AutorunPolicyModuleActivator : public BundleActivator
    {
    public:
        void Start(BundleContext context)
        {
            auto guiService = std::make_shared<StartWizard>(context);
            ServiceProperties props;
            props["Type"] = std::string("StartWizard");
            context.RegisterService<IStartWizard>(guiService, props);
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(AutorunPolicyModuleActivator)
