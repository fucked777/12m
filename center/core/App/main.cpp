//#include "FrameworkWindow.h"
#include "CoreUtility.h"
#include "CppMicroServicesUtility.h"
#include "MenuConfigLoad.h"
#include "ModulConfigLoad.h"
#include "ModulInstall.h"
#include "PlatformConfigLoad.h"
#include "PlatformInterface.h"
#include "RibbonMainWindow.h"
#include "SingleModuleLoad.h"
#include "StartWizard.h"
#include "WizardLoad.h"
#include "qtsingleapplication.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

/*
 * 0x01 载入配置文件,必须在QtSingleApplication之前加载
 * 0x02 载入向导界面,可以没有
 * 0x03 模块加载
 * 0x03 载入启动界面,可以没有
 * 0x04 载入主界面,可以没有
 * 0x05 模块卸载
 * 0x05 载入结束界面,可以没有
 */
using namespace cppmicroservices;

static QList<MenuInfo> loadMenuConfig();
int main(int argc, char* argv[])
{
    PlatformConfigInfo platformConfigInfo;           /* 平台的配置信息 */
    QList<ModuleInfo> moduleInfoList;                /* 当前所有模块的信息 */
    ServiceFrameworkContext serviceFrameworkContext; /* 当前服务的上下文 */

    /* 0x01 载入配置文件,必须在QtSingleApplication之前加载 */
    {
        PlatformConfigLoad config;
        if (!config.load())
        {
            QApplication application(argc, argv);
            QMessageBox::information(nullptr, "平台配置加载错误", config.errMsg(), "确定");
            return 1;
        }
        platformConfigInfo = config.platformConfigInfo();
    }
    /* 0x01 End */
    QtSingleApplication singleApplication(platformConfigInfo.mainInfo.instanceID, argc, argv);
    if (singleApplication.isRunning())
    {
        QMessageBox::information(nullptr, "提示", "当前程序已运行", "确定");
        return 0;
    }

    /* 0x02 载入向导界面,可以没有,即使向导错误也可以使用默认的配置运行 */
    {
        WizardLoad wizardLoad;
        auto result = wizardLoad.loadWizard(platformConfigInfo);
        switch (result)
        {
        case WizardInstallResult::Finish: /* 完成 */
        {
            platformConfigInfo = wizardLoad.platformConfigInfo();
            break;
        }
        case WizardInstallResult::Skip: /* 跳过向导 */
        {
            break;
        }
        case WizardInstallResult::Error: /* 向导加载错误 */
        {
            QMessageBox::warning(nullptr, "加载向导错误", wizardLoad.errMsg(), "确定");
            break;
        }
        case WizardInstallResult::Quit: /* 退出程序 */
        {
            return 0;
        }
        }

        /* 写入全局配置数据 */
        CoreUtility::writeConfigToGlobal(platformConfigInfo);
    }
    /* 0x02 End */

    /* 到这里是所有的配置数据已经加载完成了 */
    /* 0x03 模块加载 */
    {
        /* 3.1 模块列表加载 */
        ModulConfigLoad modulConfigLoad;
        if (!modulConfigLoad.load())
        {
            QMessageBox::critical(nullptr, "模块配置载入失败", modulConfigLoad.errMsg());
            return 1;
        }
        moduleInfoList = modulConfigLoad.moduleList();

        /* 3.2 初始化服务 */
        QString errMsg;
        serviceFrameworkContext = CoreUtility::createFrameworkContextInfo(errMsg);
        if (!serviceFrameworkContext.context)
        {
            QMessageBox::information(nullptr, "服务初始化错误", errMsg);
            return 1;
        }
        /* 3.3 启动启动界面 */
        StartWizard startWidget;
        auto result = startWidget.loadWizard(platformConfigInfo);
        switch (result)
        {
        case WizardInstallResult::Finish: /* 完成 */
        case WizardInstallResult::Skip:   /* 跳过向导 */
        {
            break;
        }
        case WizardInstallResult::Error: /* 向导加载错误 */
        {
            QMessageBox::warning(nullptr, "加载启动向导错误", startWidget.errMsg(), "确定");
            return 0;
        }
        case WizardInstallResult::Quit: /* 退出程序 */
        {
            return 0;
        }
        }
        /* 3.3 安装模块 */
        ModulInstall modulInstall(serviceFrameworkContext.context);
        QObject::connect(&modulInstall, &ModulInstall::sg_serviceInstallStatusChange, &startWidget, &StartWizard::serviceInstallStatusChange);
        auto installResult = modulInstall.install(moduleInfoList, platformConfigInfo);
        if (InstallResult::Quit == installResult)
        {
            startWidget.setControlPowerThis(true);
            return singleApplication.exec();
        }
    }
    /* 0x03 载入启动界面,可以没有*/
    /* 0x03 End */

    /* 0x04 载入主界面,可以没有 */
    /* 0x04 End */

    /* 0x05 载入结束界面,可以没有 */
    /* 0x05 模块卸载 */
    /* 0x05 End */

    //    FrameworkWindow f(context);
    //    f.setModuleInfo(loadModuleConfig());
    //    f.setMenuInfo(loadMenuConfig());
    //    f.setWindowTitle(config.platformMainConfig().name);
    //    f.init();
    //    f.showMaximized();

    RibbonMainWindow r(serviceFrameworkContext.context);
    r.setModuleInfo(moduleInfoList);
    r.setMenuInfo(loadMenuConfig());
    r.setWindowTitle(platformConfigInfo.mainInfo.name);
    r.init();
    r.showMaximized();
    r.login();

    auto ret = singleApplication.exec();
    r.closeApplication();

    /* 停止模块 */
    auto bundles = serviceFrameworkContext.context.GetBundles();
    for (auto& item : bundles)
    {
        item.Stop();
    }
    /* 删除平台的对象
     * 20210810 本来程序到这里了等程序生命周期结束自动析构就行了
     * 但是在windows上出现个问题
     * 假如加入的对象依赖了系统的库 然后在释放的时候 因为这个东东是static的
     * 他两个谁先释放不知道 然后系统库先释放的话就程序GG
     * 例子 zmq的Context
     */
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->removeAllObj();
    return ret;
}

/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
static QList<MenuInfo> loadMenuConfig()
{
    /* 加载菜单信息 */
    MenuConfigLoad menuConfigLoad;
    if (!menuConfigLoad.load())
    {
        QMessageBox::critical(nullptr, "菜单配置载入失败", menuConfigLoad.errMsg(), "确定");
        return {};
    }
    return menuConfigLoad.menuList();
}
