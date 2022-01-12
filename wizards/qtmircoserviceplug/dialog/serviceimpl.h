%{Cpp:LicenseTemplate}\
@if '%{Cpp:PragmaOnce}'
#pragma once
@else
#ifndef %{ClientServiceGUARD}
#define %{ClientServiceGUARD}
@endif

#include "INetMsg.h"
namespace cppmicroservices
{
class BundleContext;
}

class %{ClientServiceName}Impl;
class %{ClientServiceName} : public INetMsg
{

public:
    %{ClientServiceName}(cppmicroservices::BundleContext context);
    ~%{ClientServiceName}();

private:
    %{ClientServiceName}Impl *m_impl;

};
%{JS: Cpp.closeNamespaces('%{ClientServiceName}')}\
@if ! '%{Cpp:PragmaOnce}'
#endif // %{ClientServiceGUARD}
@endif
 