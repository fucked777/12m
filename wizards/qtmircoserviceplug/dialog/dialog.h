%{Cpp:LicenseTemplate}\
@if '%{Cpp:PragmaOnce}'
#pragma once
@else
#ifndef %{GUARD}
#define %{GUARD}
@endif
%{JS: QtSupport.qtIncludes([ 'QtGui/%{BaseClassDialog}' ], [ 'QtWidgets/%{BaseClassDialog}' ]) }\
%{JS: Cpp.openNamespaces('%{Class}')}\
@if %{GenerateForm}

@if ! %{JS: Cpp.hasNamespaces('%{Class}')}
QT_BEGIN_NAMESPACE
@endif
namespace Ui { class %{CN}; }
@if ! %{JS: Cpp.hasNamespaces('%{Class}')}
QT_END_NAMESPACE
@endif
@endif

namespace cppmicroservices
{
class BundleContext;
}

class %{CN}Impl;
class %{CN} : public %{BaseClassDialog}
{
    Q_OBJECT

public:
    %{CN}(cppmicroservices::BundleContext context, QWidget *parent = nullptr);
    ~%{CN}();
private:
    %{CN}Impl *m_impl;
@if %{GenerateForm}
    Ui::%{CN} *ui;
@endif
};
%{JS: Cpp.closeNamespaces('%{Class}')}\
@if ! '%{Cpp:PragmaOnce}'
#endif // %{GUARD}
@endif
