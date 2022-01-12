#ifndef MODULCONFIGLOAD_H
#define MODULCONFIGLOAD_H
#include "CoreUtility.h"
#include <QList>
#include <QString>

/*  模块配置加载 */
class ModulConfigLoad
{
public:
    ModulConfigLoad();
    ~ModulConfigLoad();

    bool load();
    QString errMsg() { return m_errMsg; }
    inline QList<ModuleInfo> moduleList() { return m_moduleList; }

private:
    QString m_errMsg;
    QList<ModuleInfo> m_moduleList;
};

#endif  // PLATFORMCONFIGLOAD_H
