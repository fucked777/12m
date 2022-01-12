#ifndef AUTOMATETESTHELPER_H
#define AUTOMATETESTHELPER_H
#include "AutomateTestDefine.h"
#include "Utility.h"

enum class AutoTestSpin
{
    Left,
    Right,
};
class AutomateTestBase;
class SingleCommandHelper;
struct ConfigMacroData;
struct DeviceID;
struct ParamMacroData;
class AutomateTestHelper
{
public:
    /* 获取旋向1左2右 */
    static Optional<AutoTestSpin> getSpin(const QString& taskCode, SystemWorkMode workMode, int dpNum);
    static Optional<std::tuple<AutoTestSpin, ConfigMacroData>> getSpinAndConfigMacroData(const QString& taskCode, SystemWorkMode workMode, int dpNum);

    /* 将测试信号接入测试计算机
     * 后端的频谱仪
     */
    /* 下行 */
    static bool backEndSignalToTestComputerDown(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);
    /* 小环 */
    static bool backEndSignalToTestComputerXH(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);
    /* 将测试信号接入测试计算机
     * 前端的频谱仪
     */
    static bool frontEndSignalToTestComputer(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);
    /* 后端测试信号选择开关切换 下行 */
    static bool backEndSwitchChangeDown(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);
    /* 后端测试信号选择开关切换 上行 */
    static bool backEndSwitchChangeUp(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);
    /* 前端测试信号选择开关切换 这里是射频信号 */
    static bool frontRadioEndSwitchChange(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);

    /* 日志 */
    static QString toLogString(const AutomateTestResultGroup&);
    /* 校零变频器关电 */
    static void xlbpqPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);
    /* 高功放关电 */
    static void hpPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self, const DeviceID& hpID);
    static void hpPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self);

    /* 获取当前在线ACU */
    static Optional<DeviceID> getOnlineACU();

    /* 发射分系统的链路
     * 设备太少了
     */
    static bool sendSystemLink(AutomateTestBase* self);
};

#endif  // AUTOMATETESTBASE_H
