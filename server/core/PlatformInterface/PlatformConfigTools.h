#ifndef PLATFORMCONFIGTOOLS_H
#define PLATFORMCONFIGTOOLS_H

#include "PlatformGlobal.h"
#include <QObject>
#include <QString>

/*
 * 平台配置工具
 */
struct PlatformConfigInfo;
class CoreUtility;
class PLATFORMSHARED_EXPORT PlatformConfigTools
{
    friend CoreUtility;

public:
    /* 平台加载的插件的目录 此为特殊目录现在是不能更改的是写死的就是运行目录下的plugins目录下 */
    static QString plugDir();
    /* 获取插件的路径 这个插件是默认的动态库
     * 后缀 windows下是dll  linux下是so
     * 前缀 windows 无  linux lib
     */
    static QString plugPath(const QString& name);
    static QString plugPath(const QString& name, const QString& prefix, const QString& suffix);

    /* 获取平台相关的配置目录 可被向导更改,但是部分配置平台已经加载更改了是无效的*/
    static QString configPlatformDir();
    /* 可以跟文件或者目录 */
    static QString configPlatform(const QString& dir);

    /* 获取业务相关的配置目录 不可被向导更改 */
    static QString configBusinessDir();
    /* 可以跟文件或者目录 */
    static QString configBusiness(const QString& subDir);

    /* 获取业务相关共享的配置目录 可被向导更改 */
    static QString configShareDir();
    /* 可以跟文件或者目录 */
    static QString configShare(const QString& file);
    /* 实例ID */
    static QString instanceID();

    /* 获取数据 */
    static QVariant value(const char* key);
    static QString valueStr(const char* key);
    static void setValue(const char* key, const QVariant&);
    /* 此函数只会改变tempMap中已有的数据,其他的不会改变 */
    static void platformConfigFromMap(PlatformConfigInfo& info, const QVariantMap& tempMap);

protected:
    static void setPlatformConfigInfo(const PlatformConfigInfo& info);
};

#endif  // PLATFORMCONFIGINFO_H
