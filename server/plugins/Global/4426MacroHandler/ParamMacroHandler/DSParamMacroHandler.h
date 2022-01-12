#ifndef DSPARAMMACROHANDLER_H
#define DSPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"

class DSParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit DSParamMacroHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    /* 点频参数 */
    bool getPointFreqParam(ParamMacroModeData&, QMap<int, QMap<int, QVariantMap>>&, const TargetInfo& targetInfo, const DeviceID& deviceID, int zp,
                           int i, int q, int mny, bool isKa);
    bool singlePointFreq(const LinkLine&, QMap<int, QMap<int, QVariantMap>>&, int zp, int i, int q, int mny, bool isKa);
    bool doublePointFreq(const LinkLine&, QMap<int, QMap<int, QVariantMap>>&, QMap<int, QMap<int, QVariantMap>>&, int zp, int i, int q, int mny,
                         bool isKa);

    bool jdParam(const LinkLine&, QMap<int, QMap<int, QVariantMap>>&, QMap<int, QMap<int, QVariantMap>>&, int zp, int i, int q, int mny, bool isKa);
    bool jdParam(int zp, int i, int q, int mny, bool isKa);
    bool kajdParam();
    bool xjdParam();
    bool kaxjdParam();

    bool ckqdParam(const ParamMacroModeData& masterTargetParamMacro);
    bool kadsgpxParam(const ParamMacroModeData& masterTargetParamMacro);
    bool xgpxParam(const ParamMacroModeData& masterTargetParamMacro);

    bool kafsgpxParam(const ParamMacroModeData& masterTargetParamMacro);
    bool kagfParam(const ParamMacroModeData& masterTargetParamMacro);

private:
    MasterSlaveDeviceIDInfo m_dsjdInfo;              // 低速基带
    MasterSlaveDeviceIDInfo m_ckqdInfo;              // 测控前端
    ParamMacroModeData m_paramMacroModeDataMaster;   // 主跟的参数 没有主跟就随便一个
    ParamMacroModeData m_xParamMacroModeDataMaster;  // X的
};

#endif  // DSPARAMMACROHANDLER_H
