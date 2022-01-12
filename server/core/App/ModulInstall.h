#ifndef MODULINSTALL_H
#define MODULINSTALL_H
#include "CoreUtility.h"
#include "CppMicroServicesUtility.h"
#include "PlatformInterface.h"
#include <QObject>
#include <QString>
enum class InstallResult
{
    Finish, /* 安装完成 */
    Quit,   /* 退出 */
};

class ModulInstall : public QObject
{
    Q_OBJECT
public:
    ModulInstall(cppmicroservices::BundleContext context);
    ~ModulInstall();
    InstallResult install(const QList<ModuleInfo>&, const PlatformConfigInfo& configInfo);
    QString errMsg() const { return m_errMsg; }

signals:
    /* 安装的服务的状态改变 */
    void sg_serviceInstallStatusChange(ModuleInstallStatus status, const QString& name, const QString& msg = QString());

private:
    cppmicroservices::BundleContext m_context;
    cppmicroservices::ListenerToken m_listenerToken;
    QString m_errMsg;
};

#endif  // MODULINSTALL_H
