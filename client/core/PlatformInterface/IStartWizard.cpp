#include "IStartWizard.h"
#include <QEvent>

IStartWizard::IStartWizard(QWidget* parent)
    : QWidget(parent)
{
    qRegisterMetaType<ModuleInstallStatus>("ModuleInstallStatus&");
    qRegisterMetaType<ModuleInstallStatus>("ModuleInstallStatus");
    qRegisterMetaType<ModuleInstallStatus>("const ModuleInstallStatus&");
    setControlPowerThis(false);
}
IStartWizard::~IStartWizard() {}
void IStartWizard::setControlPowerThis(bool controlPower)
{
    if (controlPower)
    {
        setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

        return;
    }

    setWindowFlags(Qt::Popup | Qt::CustomizeWindowHint);
}
