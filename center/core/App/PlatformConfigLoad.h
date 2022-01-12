#ifndef PLATFORMCONFIGLOAD_H
#define PLATFORMCONFIGLOAD_H
#include "PlatformDefine.h"
#include <QString>

/*
 * 此类会把所有配置全部写入qt平台的属性中
 * 可直接通过属性读取也可以通过PlatformTools读取
 * 即使加载失败也会有一个默认值
 *
 * 但是扩展配置失败就没有的
 */
class PlatformConfigLoad
{
public:
    PlatformConfigLoad();
    ~PlatformConfigLoad();
    bool load();
    QString errMsg() const { return m_errMsg; }
    PlatformConfigInfo platformConfigInfo() const { return m_info; }

private:
    PlatformConfigInfo m_info;
    QString m_errMsg;
};

#endif  // PLATFORMCONFIGLOAD_H
