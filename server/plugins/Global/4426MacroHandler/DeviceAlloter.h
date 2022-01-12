#ifndef DEVICEALLOTER_H
#define DEVICEALLOTER_H

#include "PlanRunMessageDefine.h"

struct ManualMessage;
class DeviceAlloter
{
public:
    DeviceAlloter();
    bool allot(ManualMessage& msg, QString& errorMsg);

private:
    // 设置一些链路信息到ConfigMacroData中
    void setLinkInfo(const QMap<SystemWorkMode, LinkLine>& linkLineMap, ConfigMacroData& configMacroData);

    // 获取基带配置，初始化基带配置
    bool initResourceData(QString& errorMsg);
    // 获取配置宏
    bool getConfigMacro(ConfigMacroData& configMacroData);
    // 分配测控基带
    bool allotCKJD(ConfigMacroData& configMacroData, QString& errorMsg);
    bool allotCKJD4424(const QSet<SystemWorkMode>&, ConfigMacroData& configMacroData, QString& errorMsg);
    bool allotCKJD4426(const QSet<SystemWorkMode>&, ConfigMacroData& configMacroData, QString& errorMsg);
    // 分配高速
    bool allotGSJD(ConfigMacroData& configMacroData, QString& errorMsg);
    // 分配低速基带
    bool allotDSJD(ConfigMacroData& configMacroData, QString& errorMsg);
    // 分配测控前端
    bool allotCKQD(ConfigMacroData& configMacroData, QString& errorMsg);
    // 跟踪前端跟踪基带分配
    bool allotGZ(ConfigMacroData& configMacroData, QString& errorMsg);
    // 高速DC
    bool allotGSDC(ConfigMacroData& configMacroData, QString& /*errorMsg*/);
    // 存转
    bool allotCCZF(ConfigMacroData& configMacroData, QString& /*errorMsg*/);

private:
    ManualMessage mManualMsg;
    QMap<SystemWorkMode, ResourceRestructuringData> mResourceDataMap;
};

#endif  // DEVICEALLOTER_H
