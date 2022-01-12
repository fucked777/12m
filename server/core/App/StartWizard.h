#ifndef STARTWIZARD_H
#define STARTWIZARD_H
#include "PlatformDefine.h"
#include <QObject>
#include <QString>
#include <QWidget>

class StartWizardImpl;
class StartWizard : public QObject
{
    Q_OBJECT
public:
    StartWizard();
    ~StartWizard();
    WizardInstallResult loadWizard(const PlatformConfigInfo& info);
    QString errMsg() const;

    /* 安装的服务的状态改变 */
    void serviceInstallStatusChange(ModuleInstallStatus status, const QString& name, const QString& msg = QString());
    /* 当前向导获取控制权 */
    void setControlPowerThis(bool controlPower);

private:
    StartWizardImpl* m_impl;
};

#endif  // EXECSTARTWIZARD_H
