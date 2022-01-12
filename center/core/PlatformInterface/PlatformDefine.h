#ifndef PLATFORMDEFINE_H
#define PLATFORMDEFINE_H
#include <QMap>
#include <QString>
#include <QVariant>

/* 平台UI配置 */
struct PlatformUIConfig
{
    QString wizards;    /* 向导模块 */
    QString start;      /* 启动界面 */
    QString end;        /* 退出界面 */
    QString mainWindow; /* 主界面 */
};
/* 模块安装的状态 */
enum class ModuleInstallStatus
{
    Success,  /* 安装成功 */
    Ing,      /* 正在安装 */
    NotFount, /* 未找到 */
    Skip,     /* 跳过安装 */
    Error,    /* 安装错误 */
};
class ModuleInstallStatusHelper
{
public:
    inline static QString toDesc(ModuleInstallStatus status)
    {
        switch (status)
        {
        case ModuleInstallStatus::Success:
        {
            return "安装成功";
        }
        case ModuleInstallStatus::Ing:
        {
            return "正在安装";
        }
        case ModuleInstallStatus::NotFount:
        {
            return "未找到";
        }
        case ModuleInstallStatus::Skip:
        {
            return "跳过安装";
        }
        case ModuleInstallStatus::Error:
        {
            return "安装错误";
        }
        }
        return "未知";
    }
};

/* 平台的主配置 */
struct PlatformMainConfig
{
    QString name;                            /* 程序名 */
    QString instanceID;                      /* 用作程序的实例ID(单例) */
    QString platformConfigDir;               /* 平台配置文件路径 */
    QString businessConfigDir;               /* 私有业务配置文件路径 */
    QString shareConfigDir;                  /* 公共的业务配置文件路径 */
    bool serviceInstallErrorIsExit{ false }; /* 当服务安装错误时,是否退出,这里的错误就是字面意思,不存在或者未启用都不算错误 */
};
/* 平台扩展配置 */
using PlatformExtendedConfig = QMap<QString, QVariant>;

struct PlatformConfigInfo
{
public:
    PlatformMainConfig mainInfo;
    PlatformExtendedConfig extendedInfo;
    PlatformUIConfig uiInfo;
};

/* 向导的安装结果 */
enum class WizardInstallResult
{
    Finish, /* 完成 */
    Error,  /* 向导加载错误 */
    Skip,   /* 跳过向导 */
    Quit,   /* 退出程序 */
};
#endif  // PLATFORMDEFINE_H
