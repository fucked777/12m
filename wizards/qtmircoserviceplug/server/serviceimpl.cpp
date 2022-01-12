%{Cpp:LicenseTemplate}\
#include "%{JS: Util.relativeFilePath('%{Path}/%{HdrFileName}', '%{Path}' + '/' + Util.path('%{SrcFileName}'))}"
#include "CppMicroServicesUtility.h"

%{JS: Cpp.openNamespaces('%{Class}')}\
class %{CN}Impl
{
public:
    cppmicroservices::BundleContext context;
};
%{CN}::%{CN}(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new %{CN}Impl)
{
    m_impl->context=context;
    //registerSubscribe("key", &类名::函数名,this);
}

%{CN}::~%{CN}()
{
    delete m_impl;
}

%{JS: Cpp.closeNamespaces('%{Class}')}\
