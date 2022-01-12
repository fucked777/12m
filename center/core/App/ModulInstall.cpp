#include "ModulInstall.h"
#include "CppMicroServicesUtility.h"
#include "PlatformInterface.h"
#include <QApplication>
#include <QDebug>
#include <QFile>

ModulInstall::ModulInstall(cppmicroservices::BundleContext context)
    : m_context(context)
{
    Q_ASSERT(m_context);
    m_listenerToken = m_context.AddServiceListener([=](const cppmicroservices::ServiceEvent& event) {
        std::string objectClass = cppmicroservices::ref_any_cast<std::vector<std::string>>(
                                      event.GetServiceReference().GetProperty(cppmicroservices::Constants::OBJECTCLASS))
                                      .front();
        auto curType = event.GetType();

        if (curType == cppmicroservices::ServiceEvent::SERVICE_REGISTERED)
        {
            auto msg = QString("服务安装:   %1").arg(QString::fromStdString(objectClass));
            qWarning() << msg;
        }
        else if (curType == cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING)
        {
            auto msg = QString("服务卸载:   %1").arg(QString::fromStdString(objectClass));
            qWarning() << msg;
        }
        else if (curType == cppmicroservices::ServiceEvent::SERVICE_MODIFIED)
        {
            auto msg = QString("服务更改:   %1").arg(QString::fromStdString(objectClass));
            qWarning() << msg;
        }
    });
}
ModulInstall::~ModulInstall() {}
InstallResult ModulInstall::install(const QList<ModuleInfo>& moduleInfo, const PlatformConfigInfo& configInfo)
{
    bool errorIsExit = configInfo.mainInfo.serviceInstallErrorIsExit;
    for (auto& moduleInfos : moduleInfo)
    {
        QApplication::processEvents();
        if (!moduleInfos.use)
        {
            emit sg_serviceInstallStatusChange(ModuleInstallStatus::Skip, moduleInfos.name);
            continue;
        }
        auto& moduleName = moduleInfos.name;
        auto moduleFullPath = PlatformConfigTools::plugPath(moduleName);
        /* 插件不存在 */
        if (!QFile::exists(moduleFullPath))
        {
            emit sg_serviceInstallStatusChange(ModuleInstallStatus::NotFount, moduleInfos.name);
            continue;
        }

        emit sg_serviceInstallStatusChange(ModuleInstallStatus::Ing, moduleInfos.name);
        try
        {
            auto newBundles = m_context.InstallBundles(moduleFullPath.toStdString());
            for (auto& newBundless : newBundles)
            {
                newBundless.Start();
            }
        }
        catch (const std::exception& ex)
        {
            emit sg_serviceInstallStatusChange(ModuleInstallStatus::Error, moduleInfos.name, QString(ex.what()));
            if (errorIsExit)
            {
                return InstallResult::Quit;
            }
            continue;
        }
        catch (...)
        {
            emit sg_serviceInstallStatusChange(ModuleInstallStatus::Error, moduleInfos.name, "未知错误");
            if (errorIsExit)
            {
                return InstallResult::Quit;
            }
            continue;
        }
        emit sg_serviceInstallStatusChange(ModuleInstallStatus::Success, moduleInfos.name);
    }
    return InstallResult::Finish;
}
