#ifndef KAGSPARAMMACROHANDLER_H
#define KAGSPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"
#include "SatelliteManagementDefine.h"

// Ka高速参数宏
class KaGSParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit KaGSParamMacroHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    bool kaGSJDParam();
    /* 注意这里要考虑双点频单旋向 双点频双旋向 */
    bool singlePointFreq(const MasterSlaveDeviceIDInfo& gsjdInfo);
    bool doublePointFreq(const MasterSlaveDeviceIDInfo& gsjdInfo);
    // Ka高速数传下变频器
    bool kaGSXBPQParam();
    // Ka低速数传及跟踪高频箱
    bool kaDSSCGZGPXParam(const ParamMacroModeData& masterTargetParamMacro);

    //处理特殊参数
    void dealWithSpcialParam(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap);
    void gsMNY(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap);

private:
    /* 跟踪的信息 */
    ParamMacroModeData m_paramMacroModeData;
    /* 两个点频的参数宏信息 */
    ParamMacroModeData m_paramMacroModeData1;
    ParamMacroModeData m_paramMacroModeData2;
    //两个点频的上下行频率
    QVariant m_downFreq1;  //点频1的下行频率
    QVariant m_downFreq2;  //点频2的下行频率
};
#endif  // KAGSPARAMMACROHANDLER_H
