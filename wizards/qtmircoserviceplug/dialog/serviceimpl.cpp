%{Cpp:LicenseTemplate}\
#include "%{JS: Util.relativeFilePath('%{Path}/%{ClientServiceHdr}', '%{Path}' + '/' + Util.path('%{SrcFileName}'))}"
#include "CppMicroServicesUtility.h"

%{JS: Cpp.openNamespaces('%{ClientServiceName}')}\
class %{ClientServiceName}Impl
{
public:
    cppmicroservices::BundleContext context;
};
%{ClientServiceName}::%{ClientServiceName}(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new %{ClientServiceName}Impl)
{
    m_impl->context=context;
    //registerSubscribe("key", &类名::函数名,this);
}

%{ClientServiceName}::~%{ClientServiceName}()
{
    delete m_impl;
}

%{JS: Cpp.closeNamespaces('%{ClientServiceName}')}\
