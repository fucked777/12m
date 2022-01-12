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

protected:
    // 组包组参数设置
    bool packGroupParamSetData(const DeviceID& deviceID, int modeId, const QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap,
                               PackCommand& packCommand) const;

    // 获取参数宏的点频参数
    bool getPointFreqParamMap(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, const DeviceID& deviceID,
                              QMap<int, QMap<QString, QVariant>>& unitParamMap);

    // 获取测控基带参数，根据任务代号、点频等信息把参数宏点频参数转为  QMap<单元id, QMap<目标id, QMap<参数id, 参数值>>>
    bool getUnitTargetParamMap(const DeviceID& deviceID, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);

    // 添加设备参数到要下发的参数中
    void appendDeviceParam(const QMap<int, QMap<QString, QVariant>>& deviceUnitParamMap,
                           QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);

    /* 获取参数宏数据 20210826 wp?? */
    bool getParamMacroModeData(const QString& taskCode, SystemWorkMode workMode, ParamMacroModeData& paramMacroModeItem);
};

#endif  // BASEPARAMMACROHANDLER_H
