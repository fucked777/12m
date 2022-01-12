#include "CoreUtility.h"
#include "PlatformConfigTools.h"
#include "PlatformInterface.h"
#include <QDir>
#include <QRegularExpression>

QString CoreUtility::absolutePathNativeSeparators(const QString& path)
{
    /* 先全部变成/  然后替换 */
    auto tempPath = QDir::fromNativeSeparators(path);
    tempPath.replace(QRegularExpression("/+"), "/");
    QDir tempDir(tempPath);
    return tempDir.absolutePath();
}

void CoreUtility::writeConfigToGlobal(const PlatformConfigInfo& bak)
{
    auto info = bak;
    /* 这里要处理一下路径 */
    auto& mainConfig = info.mainInfo;
    mainConfig.businessConfigDir = CoreUtility::absolutePathNativeSeparators(mainConfig.businessConfigDir);
    mainConfig.platformConfigDir = CoreUtility::absolutePathNativeSeparators(mainConfig.platformConfigDir);
    mainConfig.shareConfigDir = CoreUtility::absolutePathNativeSeparators(mainConfig.shareConfigDir);

    PlatformConfigTools::setPlatformConfigInfo(info);
}

ServiceFrameworkContext CoreUtility::createFrameworkContextInfo(QString& errMsg)
{
    ServiceFrameworkContext serviceFrameworkContext;

    try
    {
        /* 无法初始化会抛异常 */
        serviceFrameworkContext.framework.Init();
        /*
         * 未初始化抛异常
         * 无法启动抛异常
         * 已卸载抛异常
         */
        serviceFrameworkContext.framework.Start();
    }
    catch (const std::exception& e)
    {
        errMsg = QString("服务初始化错误:%1").arg(QString::fromStdString(e.what()));
        return serviceFrameworkContext;
    }
    catch (...)
    {
        errMsg = QString("服务初始化错误");
        return serviceFrameworkContext;
    }

    serviceFrameworkContext.context = serviceFrameworkContext.framework.GetBundleContext();
    if (!serviceFrameworkContext.context)
    {
        errMsg = QString("服务初始化错误:无效的Context");
    }
    return serviceFrameworkContext;
}
MenuWidgetType MenuInfoHelper::fromStr(const QString& raw)
{
    auto type = raw.toUpper();
    if (type == "DIALOG")
    {
        return MenuWidgetType::Dialog;
    }
    else if (type == "DOCK")
    {
        return MenuWidgetType::Dock;
    }
    return MenuWidgetType::UnknownType;
}
QString MenuInfoHelper::toStr(MenuWidgetType type)
{
    switch (type)
    {
    case MenuWidgetType::UnknownType:
    {
        break;
    }
    case MenuWidgetType::Dialog:
    {
        return "Dialog";
    }
    case MenuWidgetType::Dock:
    {
        return "Dock";
    }
    }
    return "Unknown";
}
