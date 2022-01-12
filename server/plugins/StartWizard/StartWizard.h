#ifndef STARTWIZARD_H
#define STARTWIZARD_H
#include "IStartWizard.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class StartWizard;
}
namespace cppmicroservices
{
    class BundleContext;
}
QT_END_NAMESPACE

class StartWizard : public IStartWizard
{
    Q_OBJECT

public:
    StartWizard(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~StartWizard();

    void serviceInstallStatusChange(ModuleInstallStatus status, const QString& name, const QString& msg) override;

private:
    Ui::StartWizard* ui;
};
#endif  // StartWizard_H
