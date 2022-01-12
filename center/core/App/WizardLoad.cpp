#include "WizardLoad.h"
#include "IWizard.h"
#include "SingleModuleLoad.h"
#include <QDialog>

WizardLoad::WizardLoad() {}
WizardLoad::~WizardLoad() {}

WizardInstallResult WizardLoad::loadWizard(const PlatformConfigInfo& info)
{
    /* 因为已经写入了全局配置所以可以直接使用平台的配置工具读取
     * 向导可以为空,但是不能写了是无效的
     */
    if (info.uiInfo.wizards.isEmpty())
    {
        return WizardInstallResult::Skip;
    }

    SingleModuleLoad load;
    if (!load.loadModule(info.uiInfo.wizards))
    {
        m_errMsg = load.errMsg();
        return WizardInstallResult::Error;
    }
    auto widget = load.instance<IWizard>();
    if (widget == nullptr)
    {
        m_errMsg = "向导模块加载错误";
        return WizardInstallResult::Error;
    }
    if (!widget->init(info))
    {
        m_errMsg = widget->errMsg();
        return WizardInstallResult::Error;
    }

    auto result = widget->exec();
    m_info = widget->platformConfigInfo();

    return QDialog::Accepted == result ? WizardInstallResult::Finish : WizardInstallResult::Quit;
}
