#ifndef COREUTILITY_H
#define COREUTILITY_H
#include "CppMicroServicesUtility.h"
#include <QList>
#include <QSize>
#include <QString>

/*
 * framework必须存在 否则context会失效
 */
struct ServiceFrameworkContext
{
    cppmicroservices::Framework framework;
    cppmicroservices::BundleContext context;

    ServiceFrameworkContext()
        : framework(getServiceFramework())
    {
    }

private:
    static cppmicroservices::Framework getServiceFramework()
    {
        cppmicroservices::FrameworkFactory factory;
        return factory.NewFramework();
    }
};

/* 模块信息 */
struct ModuleInfo
{
    QString name;
    bool use{ true }; /* 是否使用 */
};

/* 菜单widget类型 */
enum class MenuWidgetType
{
    UnknownType,
    Dialog,
    Dock,
};

/* 菜单信息 */
struct MenuInfo
{
    QString name;
    QString moduleName;
    QString subWidgetName;
    QString pngSrc;
    QString iconName;
    MenuWidgetType widgetType;
    bool isDefaultTriggered{ false };
    QSize size{ QSize(1620, 900) };

    QList<MenuInfo> subMenus;
};
class MenuInfoHelper
{
public:
    static MenuWidgetType fromStr(const QString&);
    static QString toStr(MenuWidgetType);
};

struct PlatformConfigInfo;
class CoreUtility
{
public:
    /* 统一返回 /作为路径分隔符 且转换为绝对路径 */
    static QString absolutePathNativeSeparators(const QString& path);
    /* 将配置全局写入 */
    static void writeConfigToGlobal(const PlatformConfigInfo& info);
    /* 创建CppMicroServices的框架上下文信息 */
    static ServiceFrameworkContext createFrameworkContextInfo(QString& errMsg);
};

#endif  // COREUTILITY_H
