#ifndef BASEPARAMMACROHANDLER_H
#define BASEPARAMMACROHANDLER_H

#include "BaseHandler.h"
#include "PlanRunMessageDefine.h"

class BaseParamMacroHandler : public BaseHandler
{
    Q_OBJECT
public:
    explicit BaseParamMacroHandler(QObject* parent = nullptr);

    virtual SystemWorkMode getSystemWorkMode() = 0;

    using UnitTargetParamMap = QMap<int, QMap<int, QVariantMap>>;

protected:
    // 组包组参数设置
    bool packGroupParamSetData(const DeviceID& deviceID, int modeId, const UnitTargetParamMap& unitTargetParamMap, PackCommand& packCommand) const;

    // 获取参数宏的点频参数
    bool getPointFreqAndDeviceParamMap(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, const DeviceID& deviceID,
                                       QMap<int, QVariantMap>& unitParamMap);
    bool getPointFreqAndDeviceParamMap(const ParamMacroModeData& paramMacroModeData, const QString& taskCode, SystemWorkMode workMode,
                                       int pointFreqNo, const DeviceID& deviceID, QMap<int, QVariantMap>& unitParamMap);
    // 获取测控基带参数，根据任务代号、点频等信息把参数宏点频参数转为  QMap<单元id, QMap<目标id, QMap<参数id, 参数值>>>
    bool getUnitTargetParamMap(const DeviceID& deviceID, UnitTargetParamMap& unitTargetParamMap);
    //这里加了个LinkLine参数项的原因是跟踪基带要用到
    bool getUnitTargetParamMap(SystemWorkMode workMode, LinkLine LinkLine, const DeviceID& deviceID, UnitTargetParamMap& unitTargetParamMap);

    // 添加设备参数到要下发的参数中
    void appendDeviceParam(const QMap<int, QVariantMap>& deviceUnitParamMap, UnitTargetParamMap& unitTargetParamMap);

    /* 获取参数宏数据 20210826 wp?? */
    bool getParamMacroModeData(const QString& taskCode, SystemWorkMode workMode, ParamMacroModeData& paramMacroModeItem);
    bool getParamMacroModeData(const QString& taskCode, ParamMacroData& paramMacroData);

    /* 跟踪前端下宏 */
    bool gzqdParam(const ParamMacroModeData& masterTargetParamMacro, const QVariant& down = QVariant());

    /* 获取目标对应的卫星信息 */
    bool targetInfoMapParamDown(SatelliteManagementData&, SystemOrientation&, QVariant&, QMap<int, QVariantMap>&, ParamMacroModeData&,
                                const TargetInfo& info, const DeviceID& deviceID);
};

#endif  // BASEPARAMMACROHANDLER_H
