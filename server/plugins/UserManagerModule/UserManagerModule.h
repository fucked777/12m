#ifndef USERMANAGERMODULE_H
#define USERMANAGERMODULE_H

#include "INetMsg.h"
namespace cppmicroservices
{
class BundleContext;
}

class UserManagerModuleImpl;
class UserManagerModule : public INetMsg
{

public:
    UserManagerModule(cppmicroservices::BundleContext context);
    ~UserManagerModule();

private:
    UserManagerModuleImpl *m_impl;

};
#endif // USERMANAGERMODULE_H
