%{Cpp:LicenseTemplate}\
#include "%{JS: Util.relativeFilePath('%{Path}/%{HdrFileName}', '%{Path}' + '/' + Util.path('%{SrcFileName}'))}"
#include "CppMicroServicesUtility.h"
@if %{GenerateForm}
#include "%{UiHdrFileName}"
@endif

%{JS: Cpp.openNamespaces('%{Class}')}\
class %{CN}Impl
{
public:
    cppmicroservices::BundleContext context;
};
%{CN}::%{CN}(cppmicroservices::BundleContext context, QWidget *parent)
    : %{BaseClassDialog}(parent)
    , m_impl(new %{CN}Impl)
@if %{GenerateForm}
    , ui(new Ui::%{CN})
@endif
{
    m_impl->context=context;
@if %{GenerateForm}
    ui->setupUi(this);
@endif
}

%{CN}::~%{CN}()
{
@if %{GenerateForm}
    delete ui;
@endif
    delete m_impl;
}

%{JS: Cpp.closeNamespaces('%{Class}')}\
