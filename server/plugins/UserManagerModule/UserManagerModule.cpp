#include "UserManagerModule.h"
#include "CppMicroServicesUtility.h"

class UserManagerModuleImpl
{
public:
    cppmicroservices::BundleContext context;
};
UserManagerModule::UserManagerModule(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new UserManagerModuleImpl)
{
    m_impl->context = context;
    // registerSubscribe("key", &类名::函数名,this);
}

UserManagerModule::~UserManagerModule() { delete m_impl; }
