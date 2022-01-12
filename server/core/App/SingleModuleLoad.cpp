#include "SingleModuleLoad.h"
#include "CppMicroServicesUtility.h"
#include "PlatformConfigTools.h"
#include <QDir>
#include <QString>

cppmicroservices::Framework getServiceFramework()
{
    cppmicroservices::FrameworkFactory factory;
    return factory.NewFramework();
}
SingleModuleLoad::SingleModuleLoad()
    : m_framework(getServiceFramework())
{
}
SingleModuleLoad::~SingleModuleLoad()
{
    if (!m_context)
    {
        return;
    }
    auto bundles = m_context.GetBundles();
    for (auto& item : bundles)
    {
        item.Stop();
    }
}
bool SingleModuleLoad::loadModule(const QString& moduleName)
{
    try
    {
        /* 无法初始化会抛异常 */
        m_framework.Init();
        /*
         * 未初始化抛异常
         * 无法启动抛异常
         * 已卸载抛异常
         */
        m_framework.Start();
    }
    catch (const std::exception& e)
    {
        m_errMsg = QString("服务初始化错误:%1").arg(QString::fromStdString(e.what()));
        return false;
    }
    catch (...)
    {
        m_errMsg = QString("服务初始化错误");
        return false;
    }
    auto moduleFullPath = PlatformConfigTools::plugPath(moduleName);
    /* 插件不能不存在 */
    if (!QFile::exists(moduleFullPath))
    {
        m_errMsg = QString("向导不存在:%1").arg(moduleName);
        return false;
    }

    m_context = m_framework.GetBundleContext();
    if (!m_context)
    {
        m_errMsg = QString("服务初始化错误:无效的Context");
        return false;
    }

    try
    {
        auto newBundles = m_context.InstallBundles(moduleFullPath.toStdString());
        for (auto& newBundle : newBundles)
        {
            newBundle.Start();
        }
    }
    catch (const std::exception& e)
    {
        m_errMsg = QString("安装插件失败:%1  %2").arg(moduleName, QString::fromStdString(e.what()));
        return false;
    }
    return true;
}
