#ifndef AGCCALIBRATIONHELPER_H
#define AGCCALIBRATIONHELPER_H
#include "ConfigMacroMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "SystemWorkMode.h"
#include "Utility.h"
#include <tuple>

enum class AGCCalibrationtSpin
{
    Left,
    Right,
};
class AGCCalibrationHelper
{
public:
    /* 1 左 2 右 混合左旋优先 */
    static Optional<AGCCalibrationtSpin> getSpin(const QString& taskCode, SystemWorkMode workMode, int dpNum);
    static Optional<std::tuple<AGCCalibrationtSpin, ConfigMacroData>> getSpinAndConfigMacroData(const QString& taskCode, SystemWorkMode workMode,
                                                                                                int dpNum);
    static Optional<QByteArray> cmdPack(const CmdRequest& cmdRequest);
};

#endif  // AGCCALIBRATIONHELPER_H
