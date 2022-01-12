#ifndef ISTARTWIZARD_H
#define ISTARTWIZARD_H
#include "PlatformDefine.h"
#include "PlatformGlobal.h"
#include <QWidget>

/* 向导界面的接口 */
class PLATFORMSHARED_EXPORT IStartWizard : public QWidget
{
    Q_OBJECT
public:
    IStartWizard(QWidget* parent = nullptr);
    virtual ~IStartWizard();
    /* 安装状态改变 */
    virtual void serviceInstallStatusChange(ModuleInstallStatus status, const QString& name, const QString& msg) = 0;
    /* 设置当前界面是否具有程序的控制权 */
    virtual void setControlPowerThis(bool controlPower);
};

#endif  // ISTARTWIZARD_H
