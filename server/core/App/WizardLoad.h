#ifndef WIZARDLOAD_H
#define WIZARDLOAD_H

#include "PlatformDefine.h"
#include <QString>

class WizardLoad
{
public:
    WizardLoad();
    ~WizardLoad();
    WizardInstallResult loadWizard(const PlatformConfigInfo& config); /* 加载向导 */
    /* 获取经过向导处理以后的配置数据,此数据loadWizard返回值不为Quit Error时间有效,否则数据是无效的 */
    PlatformConfigInfo platformConfigInfo() const { return m_info; }
    QString errMsg() const { return m_errMsg; }

private:
    PlatformConfigInfo m_info;
    QString m_errMsg;
};

#endif  // WIZARDLOAD_H
