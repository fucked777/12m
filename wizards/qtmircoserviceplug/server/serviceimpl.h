%{Cpp:LicenseTemplate}\
@if '%{Cpp:PragmaOnce}'
#pragma once
@else
#ifndef %{GUARD}
#define %{GUARD}
@endif

#include "INetMsg.h"
namespace cppmicroservices
{
class BundleContext;
}

class %{CN}Impl;
class %{CN} : public INetMsg
{

public:
    %{CN}(cppmicroservices::BundleContext context);
    ~%{CN}();

private:
    %{CN}Impl *m_impl;

};
%{JS: Cpp.closeNamespaces('%{Class}')}\
@if ! '%{Cpp:PragmaOnce}'
#endif // %{GUARD}
@endif
 