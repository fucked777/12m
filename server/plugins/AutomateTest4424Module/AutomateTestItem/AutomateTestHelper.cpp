#include "AutomateTestHelper.h"
#include "AutomateTest.h"
#include "AutomateTestBase.h"
#include "AutomateTestSerialize.h"
#include "BusinessMacroCommon.h"
#include "CConverter.h"
#include "ConfigMacroMessageSerialize.h"
#include "ParamMacroMessageSerialize.h"
#include "SatelliteManagementSerialize.h"
#include "SingleCommandHelper.h"

Optional<std::tuple<AutoTestSpin, ConfigMacroData>> AutomateTestHelper::getSpinAndConfigMacroData(const QString& taskCode, SystemWorkMode workMode,
                                                                                                  int dpNum)
{
    /* 旋向 */
    using ResType = Optional<std::tuple<AutoTestSpin, ConfigMacroData>>;
    auto spinResult = getSpin(taskCode, workMode, dpNum);
    if (!spinResult)
    {
        return ResType(spinResult.errorCode(), spinResult.msg());
    }

    /* 配置宏 */
    ConfigMacroData configMacroData;
    auto getDataResult = GlobalData::getConfigMacroData(workMode, configMacroData, MasterSlave::Master);
    if (!getDataResult)
    {
        return ResType(ErrorCode::InvalidData, "获取当前配置宏错误");
    }
    return ResType(std::make_tuple(spinResult.value(), configMacroData));
}
Optional<AutoTestSpin> AutomateTestHelper::getSpin(const QString& taskCode, SystemWorkMode workMode, int dpNum)
{
    using ResType = Optional<AutoTestSpin>;
    /* 旋向 */
    SystemOrientation orientation = SystemOrientation::Unkonwn;
    SatelliteManagementData satelliteTemp;
    if (!GlobalData::getSatelliteManagementData(taskCode, satelliteTemp))
    {
        return ResType(ErrorCode::InvalidArgument, "获取旋向错误:卫星数据获取错误");
    }
    auto getDataResult = satelliteTemp.getSReceivPolar(workMode, dpNum, orientation);
    if (!getDataResult)
    {
        return ResType(ErrorCode::InvalidData, QString("获取当前卫星旋向错误:%1").arg(satelliteTemp.m_satelliteName));
    }

    if (orientation == SystemOrientation::LCircumflex ||  //
        orientation == SystemOrientation::LRCircumflex)
    {
        return ResType(AutoTestSpin::Left);
    }
    else if (orientation == SystemOrientation::RCircumflex)
    {
        return ResType(AutoTestSpin::Right);
    }

    return ResType(ErrorCode::InvalidData, QString("获取当前卫星旋向错误:%1").arg(satelliteTemp.m_satelliteName));
}

/* 将测试信号接入测试计算机 */
bool AutomateTestHelper::backEndSignalToTestComputerDown(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /* 链路切换完成后，还需要把信号接入测试计算机 下行信号 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        /*
         * S频段 左旋的话是1接25 右旋是2接25
         * S频段接收开关矩阵
         */
        QVariantMap replace;
        replace["ChooseConnect_25"] = (self->isLeftSpin() ? 1 : 2);
        singleCmdHelper.packSingleCommand("Step_RFSM_S_DownlinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (workMode == SystemWorkMode::KaKuo2)
    { /*
       * Ka遥测 左旋的话是6接25 右旋是7接25
       * S频段接收开关矩阵
       */
        QVariantMap replace;
        replace["ChooseConnect_25"] = (self->isLeftSpin() ? 6 : 7);
        singleCmdHelper.packSingleCommand("Step_RFSM_KaYC_DownlinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (workMode == SystemWorkMode::KaDS)
    {
        /*
         * Ka低速 左旋的话是12接25 右旋是13接25
         * S频段接收开关矩阵
         */
        QVariantMap replace;
        replace["ChooseConnect_25"] = (self->isLeftSpin() ? 12 : 13);
        singleCmdHelper.packSingleCommand("Step_RFSM_KaDS_DownlinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (workMode == SystemWorkMode::KaGS)
    {
        /*
         * Ka高速 没有走S频段接收开关矩阵是走的1.2g中频开关矩阵
         * 左旋 1接6 右旋 2接6
         */
        QVariantMap replace;
        replace["ChooseConnect_6"] = (self->isLeftSpin() ? 1 : 2);
        singleCmdHelper.packSingleCommand("Step_LSFBRFSMJ_DownlinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}
bool AutomateTestHelper::backEndSignalToTestComputerXH(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /* 链路切换完成后，还需要把信号接入测试计算机 小环信号 */

    // 9  Ka测控小环
    // 10 Ka数传小环
    // 11 S测控小环
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        QVariantMap replace;
        replace["ChooseConnect_12"] = 11;
        singleCmdHelper.packSingleCommand("Step_SFCLRFSM_S_UplinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (workMode == SystemWorkMode::KaKuo2)
    {
        QVariantMap replace;
        replace["ChooseConnect_12"] = 9;
        singleCmdHelper.packSingleCommand("Step_SFCLRFSM_S_UplinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (workMode == SystemWorkMode::KaDS)
    {
        QVariantMap replace;
        replace["ChooseConnect_12"] = 10;
        singleCmdHelper.packSingleCommand("Step_SFCLRFSM_S_UplinkTest", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}

bool AutomateTestHelper::frontEndSignalToTestComputer(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /* 链路切换完成后，还需要把信号接入测试计算机 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        /*
         * S频段 左旋的话是1 右旋是2
         * 不测试差路
         * S频段射频开关网络
         */
        QVariantMap replace;
        replace["LocalOsciTestSignChoose"] = (self->isLeftSpin() ? 1 : 2);
        singleCmdHelper.packSingleCommand("Step_SBRFSN_QDPPY", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::KaBand)
    {
        /*
         * Ka遥测 左旋的话是1 右旋是2
         * 不测试差路
         * Ka频段射频开关网络
         */
        QVariantMap replace;
        replace["Step_KaBRFSN_QDPPY"] = (self->isLeftSpin() ? 1 : 2);
        singleCmdHelper.packSingleCommand("Step_KaBRFSN_QDPPY", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::XBand)
    {
        /*
         * X 左旋的话是1 右旋是2
         * 不测试差路
         * X频段高频箱
         */
        QVariantMap replace;
        replace["LeftPolSwitch_X"] = (self->isLeftSpin() ? 1 : 2);
        singleCmdHelper.packSingleCommand("Step_XBRFSN_QDPPY", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}

bool AutomateTestHelper::backEndSwitchChangeDown(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /* 后端测试开关网络输出 除开高速选择1.2G以外都选择S1入 */
    if (workMode == SystemWorkMode::KaGS)
    {
        QVariantMap replace;
        replace["OscillChannSel1"] = 3;
        singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }

    /* 除开高速以外的下行都选择S1入 */
    QVariantMap replace;
    replace["OscillChannSel1"] = 1;
    singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
    auto execStatus = self->waitExecSuccess(packCommand, 40);
    return (execStatus == AutomateTestBase::ExecStatus::Success);
}

bool AutomateTestHelper::backEndSwitchChangeUp(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    PackCommand packCommand;
    /* 后端测试开关网络输出 全部选择S2入 */
    QVariantMap replace;
    replace["OscillChannSel1"] = 2;
    singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
    auto execStatus = self->waitExecSuccess(packCommand, 40);
    return (execStatus == AutomateTestBase::ExecStatus::Success);
}

bool AutomateTestHelper::frontRadioEndSwitchChange(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /*
     * 这里接的是射频信号
     * uvalue="1" desc="Ka测控上行"
     * uvalue="2" desc="Ka低速数传上行"
     * uvalue="3" desc="S下行"
     * uvalue="4" desc="X下行"
     * uvalue="5" desc="Ka下行"
     * uvalue="6" desc="Ka测控上行本振"
     * uvalue="7" desc="Ka低速数传上行本振"
     * uvalue="8" desc="X下行本振"
     * uvalue="9" desc="Ka测控及跟踪下行本振"
     * uvalue="10" desc="Ka低速及跟踪下行本振"
     * uvalue="11" desc="Ka高速下行本振"
     * uvalue="12" desc="10MHz频率源"
     */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        QVariantMap replace;
        replace["LanchFreqChanState"] = 3;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (workMode == SystemWorkMode::KaKuo2)
    {
        QVariantMap replace;
        replace["LanchFreqChanState"] = 5;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::XBand)
    {
        QVariantMap replace;
        replace["LeftPolSwitch_X"] = 4;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}

void AutomateTestHelper::xlbpqPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /* 校零变频器关电 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        singleCmdHelper.packSingleCommand("StepSXL_STARTQD", packCommand);
        self->notWaitExecSuccessBeQuiet(packCommand);
    }
    //    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::KaBand)
    //    {
    //        singleCmdHelper.packSingleCommand("StepKaXB_STOP", packCommand);
    //        self->notWaitExecSuccessBeQuiet(packCommand);
    //    }
}
QString AutomateTestHelper::toLogString(const AutomateTestResultGroup& group)
{
    auto msg = QString("当前测得\n");
    for (auto& item : group.resultList)
    {
        msg += QString("%1:%2\n").arg(item.displayName, item.displayValue);
    }
    if (msg.endsWith("\n"))
    {
        msg.chop(1);
    }
    return msg;
}

void AutomateTestHelper::hpPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    /* 配置宏 */
    DeviceID hpID;
    if (!self->m_configMacroData.getSGGFDeviceID(self->m_testPlanItem.workMode, hpID))
    {
        return;
    }
    hpPowerOff(singleCmdHelper, self, hpID);
}
void AutomateTestHelper::hpPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self, const DeviceID& hpID)
{
    auto workMode = self->m_testPlanItem.workMode;
    PackCommand packCommand;
    /* 功放关电 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        singleCmdHelper.packSingleCommand("Step_SHPA_POWER_OFF", packCommand, hpID);
        self->notWaitExecSuccessBeQuiet(packCommand);
    }
    else if (workMode == SystemWorkMode::KaKuo2)
    {
        singleCmdHelper.packSingleCommand("StepKaHPA_Power_Forbid", packCommand, hpID);
        self->notWaitExecSuccessBeQuiet(packCommand);
    }
    else if (workMode == SystemWorkMode::KaDS || workMode == SystemWorkMode::KaGS)
    {
        singleCmdHelper.packSingleCommand("StepKaSCHPA_Power_Forbid", packCommand, hpID);
        self->notWaitExecSuccessBeQuiet(packCommand);
    }
}

Optional<DeviceID> AutomateTestHelper::getOnlineACU()
{
    using ResType = Optional<DeviceID>;
    DeviceID acuADeviceID(0x1011);
    DeviceID acuBDeviceID(0x1012);

    if (!GlobalData::getDeviceOnline(acuADeviceID) && !GlobalData::getDeviceOnline(acuBDeviceID))
    {
        return ResType(ErrorCode::DevOffline, "ACU主备设备离线");
    }

    /*
     * 1 在线 表示当前主机
     * 2 离线 表示当前备机
     * 根据4413的情况ACU的主备是绝对的主备 不能两个同时使用的
     */
    auto tempVarA = GlobalData::getReportParamData(acuADeviceID, 0x01, "OnLineMachine");
    auto tempVarB = GlobalData::getReportParamData(acuBDeviceID, 0x01, "OnLineMachine");
    if (tempVarA.toInt() == 1)
    {
        return ResType(acuADeviceID);
    }
    else if (tempVarB.toInt() == 1)
    {
        return ResType(acuBDeviceID);
    }

    return ResType(ErrorCode::InvalidData, "未找到当前在线ACU");
}
