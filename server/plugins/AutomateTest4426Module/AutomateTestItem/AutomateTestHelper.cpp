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
    auto workMode = self->testPlanItem().workMode;
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
    auto workMode = self->testPlanItem().workMode;
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
    auto workMode = self->testPlanItem().workMode;
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
    auto workMode = self->testPlanItem().workMode;
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
    auto workMode = self->testPlanItem().workMode;
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
bool AutomateTestHelper::frontLocalVibrationEndSwitchChange(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->testPlanItem().workMode;
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
    if (workMode == SystemWorkMode::KaKuo2)
    {
        QVariantMap replace;
        replace["LanchFreqChanState"] = 9;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    if (workMode == SystemWorkMode::KaDS)
    {
        QVariantMap replace;
        replace["LanchFreqChanState"] = 10;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    if (workMode == SystemWorkMode::KaGS)
    {
        QVariantMap replace;
        replace["LanchFreqChanState"] = 11;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::XBand)
    {
        QVariantMap replace;
        replace["LeftPolSwitch_X"] = 8;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        /* S没有本振 这里是直接给下行信号就行 */
        QVariantMap replace;
        replace["LanchFreqChanState"] = 3;
        singleCmdHelper.packSingleCommand("Step_FTHSN_BandwidthSwitching", packCommand, replace);
        auto execStatus = self->waitExecSuccess(packCommand, 40);
        return (execStatus == AutomateTestBase::ExecStatus::Success);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}

void AutomateTestHelper::xlbpqPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    auto workMode = self->testPlanItem().workMode;
    PackCommand packCommand;
    /* 校零变频器关电 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::SBand)
    {
        singleCmdHelper.packSingleCommand("StepSXB_STOP", packCommand);
        self->notWaitExecSuccessBeQuiet(packCommand);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SystemBandMode::KaBand)
    {
        singleCmdHelper.packSingleCommand("StepKaXB_STOP", packCommand);
        self->notWaitExecSuccessBeQuiet(packCommand);
    }
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
    if (!self->configMacroData().getSGGFDeviceID(self->testPlanItem().workMode, hpID))
    {
        return;
    }
    hpPowerOff(singleCmdHelper, self, hpID);
}
void AutomateTestHelper::hpPowerOff(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self, const DeviceID& hpID)
{
    auto workMode = self->testPlanItem().workMode;
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

bool AutomateTestHelper::localVibrationTestSignChoose(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    self->pushLog("有疑问 Ka遥测和Ka低速高频箱 不知道输出的本振是啥", true);
    /*
     * 这里还没有去设置频率
     * 未完
     */
    return false;

    PackCommand packCommand;
    auto& m_testPlanItem = self->testPlanItem();
    if (!m_testPlanItem.testParameterMap.contains("BenZhen"))
    {
        self->pushLog("参数错误,未选择测试的本振信号", true);
        return false;
    }
    auto benzhen = m_testPlanItem.testParameterMap.value("BenZhen").rawValue.toInt();
    QVariantMap replaceParamMap;
    replaceParamMap["LocalOsciTestSignChoose"] = benzhen;
    /* 具体使用哪个本振信号 */
    if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        /* ka测控及跟踪高频箱 */
        singleCmdHelper.packSingleCommand("Step_KaCKJGZGPX_BenZhen_Change", packCommand, replaceParamMap);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }
    if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
    {
        /* ka低速数传及跟踪高频箱 */
        singleCmdHelper.packSingleCommand("Step_KaDSSCGZ_BenZhen_Change", packCommand, replaceParamMap);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }
    if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    {
        /* ka高速数传高频箱 */
        singleCmdHelper.packSingleCommand("Step_KaGSSCGPX_BenZhen_Change", packCommand, replaceParamMap);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::XBand)
    {
        /* ka高速数传高频箱 */
        singleCmdHelper.packSingleCommand("Step_XPDGPX_BenZhen_Change", packCommand, replaceParamMap);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}
bool AutomateTestHelper::frontSignalSourceToLNA(SingleCommandHelper& singleCmdHelper, AutomateTestBase* self)
{
    PackCommand packCommand;
    auto& m_testPlanItem = self->testPlanItem();
    /* 信号源接入测试高频箱 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        singleCmdHelper.packSingleCommand("Step_CSGPX_S_SignalSourceToLNA", packCommand);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::XBand)
    {
        singleCmdHelper.packSingleCommand("Step_CSGPX_X_SignalSourceToLNA", packCommand);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
    {
        singleCmdHelper.packSingleCommand("Step_CSGPX_Ka_SignalSourceToLNA", packCommand);
        return AutomateTestBase::ExecStatus::Success == self->waitExecSuccess(packCommand, 40);
    }

    self->pushLog("此测试当前不支持当前模式进行测试", true);
    return false;
}

bool AutomateTestHelper::sendSystemLink(SingleCommandHelper& singleCmdHelper, DeviceID& hp, AutomateTestBase* self)
{
    auto& testPlanItem = self->testPlanItem();
    auto& configMacroData = self->configMacroData();
    /* 切换链路
     * S
     * (基带20210831去掉由前端发送)--->前端--->功放(小环)--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
     * ka遥测/Ka低速
     * (基带20210831去掉由前端发送)--->前端--->发射高频箱--->功放(小环)--->发射高频箱--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
     */

    PackCommand packCommand;
    DeviceID ckqd; /* 前端 */
    // DeviceID hp;   /* 功放 */
    int uc{ 0 }; /* 测控/数传上变频器 */
    // ParamMacroModeData paramMacroModeItem; /* 参数宏 */

    /* 获取设备ID */
    {
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(testPlanItem.workMode) == SBand)
        {
            if (!configMacroData.getSGGFDeviceID(testPlanItem.workMode, hp))
            {
                self->pushLog("获取当前主用功放错误", true);
                return false;
            }
        }
        else if (testPlanItem.workMode == KaKuo2)
        {
            if (!configMacroData.getKaGGFDeviceID(testPlanItem.workMode, hp))
            {
                self->pushLog("获取当前主用功放错误", true);
                return false;
            }
            /* 和路上变频器 */
            auto cmdMap = configMacroData.configMacroCmdModeMap.value(testPlanItem.workMode);
            for (auto& item : cmdMap.configMacroCmdMap)
            {
                if (item.id.endsWith("UC"))
                {
                    uc = item.value.toInt();
                    break;
                }
            }
            if (uc <= 0)
            {
                self->pushLog("获取当前主用测控上变频器错误", true);
                return false;
            }
        }
        else if (testPlanItem.workMode == KaDS)
        {
            if (!configMacroData.getKaDtGGFDeviceID(testPlanItem.workMode, hp))
            {
                self->pushLog("获取当前主用功放错误", true);
                return false;
            }
            /* 和路上变频器 */
            auto cmdMap = configMacroData.configMacroCmdModeMap.value(testPlanItem.workMode);
            for (auto& item : cmdMap.configMacroCmdMap)
            {
                if (item.id.endsWith("UC"))
                {
                    uc = item.value.toInt();
                    break;
                }
            }
            if (uc <= 0)
            {
                self->pushLog("获取当前主用数传上变频器错误", true);
                return false;
            }
        }
        else
        {
            self->pushLog("当前系统工作模式不正确", true);
            return false;
        }

        if (!configMacroData.getCKQDDeviceID(testPlanItem.workMode, ckqd))
        {
            self->pushLog("获取当前使用测控前端错误", true);
            return false;
        }
        // if (!GlobalData::getParamMacroModeData(testPlanItem.taskCode, testPlanItem.workMode, paramMacroModeItem))
        // {
        //     self->pushLog("获取当前参数宏错误", true);
        //     return false;
        // }
    }

    /* 切换链路设置参数 */
    /* 前端 */
    {
        /* 发射改为前端输出 */
        QVariantMap replace;
        replace["SendFreq_2"] = self->upFreq();
        singleCmdHelper.packSingleCommand("Step_CKQD_QDSEND_AUTOTEST", packCommand, ckqd, replace);
        if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
        {
            return false;
        }
    }

    /* S频段发射开关矩阵切换 */
    {
        /*
         * 这里直接将前端的模拟源
         * 上行模拟源接所有功放
         * 下行小环接后端测试开关网络
         */

        QVariantMap replace;
        /*
         * 前端1 模拟源 2
         * 前端2 模拟源 4
         * 前端3 模拟源 6
         * 前端4 模拟源 8
         */
        auto connectPort = ckqd.extenID * 2;
        replace["ChooseConnect_1"] = connectPort;
        replace["ChooseConnect_2"] = connectPort;
        replace["ChooseConnect_3"] = connectPort;
        replace["ChooseConnect_4"] = connectPort;
        replace["ChooseConnect_5"] = connectPort;
        replace["ChooseConnect_6"] = connectPort;
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(testPlanItem.workMode) == SystemBandMode::SBand)
        {
            replace["ChooseConnect_12"] = 11;
        }
        else if (testPlanItem.workMode == SystemWorkMode::KaKuo2)
        {
            replace["ChooseConnect_12"] = 9;
        }
        else if (testPlanItem.workMode == SystemWorkMode::KaDS)
        {
            replace["ChooseConnect_12"] = 10;
        }
        singleCmdHelper.packSingleCommand("Step_SFSKGJZ_SendToHp_RecvToTest_AUTOTEST", packCommand, ckqd, replace);
        if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
        {
            return false;
        }
    }

    /* 发射高频箱 */
    {
        /*
         * 要设置3个单元的参数
         * 公共单元选    上变频器和小环
         * UC1/UC2  频率和衰减
         * 小环UC     频率和衰减
         *
         * Step_KaCKFSGPX_CommonUnit
         * Step_KaCKFSGPX_UC1Unit
         * Step_KaCKFSGPX_UC2Unit
         * Step_KaCKFSGPX_RingletUC
         *
         * Step_KaSCFSGPX_CommonUnit
         * Step_KaSCFSGPX_UC1Unit
         * Step_KaSCFSGPX_UC2Unit
         * Step_KaSCFSGPX_RingletUC
         */
        QString stepKey;
        if (testPlanItem.workMode == KaKuo2)
        {
            stepKey = "CK";
        }
        else if (testPlanItem.workMode == KaDS)
        {
            stepKey = "SC";
        }

        QVariantMap replace;
        replace["KaLanchSwitchNet"] = uc;
        singleCmdHelper.packSingleCommand(QString("Step_Ka%1FSGPX_CommonUnit").arg(stepKey), packCommand, ckqd, replace);
        if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
        {
            return false;
        }

        replace.clear();
        replace["WorkRate"] = self->upFreq();
        singleCmdHelper.packSingleCommand(QString("Step_Ka%1FSGPX_UC%2Unit").arg(stepKey).arg(uc), packCommand, ckqd, replace);
        if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
        {
            return false;
        }

        singleCmdHelper.packSingleCommand(QString("Step_Ka%1FSGPX_RingletUC").arg(stepKey), packCommand, ckqd, replace);
        if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
        {
            return false;
        }
    }

    /* 功放 */
    {
        /*
         * 功放加电
         * 输出功率50w
         * 功放去负载
         */
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(testPlanItem.workMode) == SystemBandMode::SBand)
        {
            /* 功放加电上功率 */
            singleCmdHelper.packSingleCommand("STEP_SHPA_ON_SPECIFIED_POWER_AUTOTESTLPNT", packCommand, hp);
            if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
            {
                return false;
            }

            /* 这里是主用的去负载 这里换算成备用去天线 */
            QVariantMap replaceParamMap;
            replaceParamMap["SwitcherSet"] = (hp.extenID == 1 ? 2 : 1);
            singleCmdHelper.packSingleCommand("StepGFQHKG_Signal_Destination", packCommand, hp, replaceParamMap);
            if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
            {
                return false;
            }
        }
        else if (testPlanItem.workMode == SystemWorkMode::KaKuo2)
        {
            QVariantMap replaceParamMap;
            replaceParamMap["WorkRate"] = self->upFreq();
            singleCmdHelper.packSingleCommand("StepKaHPA_Unit_AUTOTEST", packCommand, hp, replaceParamMap);
            if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
            {
                return false;
            }

            /* 这里是主用的去负载 */
            singleCmdHelper.packSingleCommand("Step5_UNGO_ACU", packCommand, hp);
            if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
            {
                return false;
            }
        }
        else if (testPlanItem.workMode == SystemWorkMode::KaDS)
        {
            QVariantMap replaceParamMap;
            replaceParamMap["WorkRate"] = self->upFreq();
            singleCmdHelper.packSingleCommand("StepKaSCHPA_Unit_AUTOTEST", packCommand, hp, replaceParamMap);
            if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
            {
                return false;
            }

            /* 这里是主用的去负载 */
            singleCmdHelper.packSingleCommand("StepKaSCHPA_UNGO_ACU", packCommand, hp);
            if (AutomateTestBase::ExecStatus::Success != self->waitExecSuccess(packCommand, 40))
            {
                return false;
            }
        }
    }

    /* 通过S2入接入频谱仪 */
    return backEndSwitchChangeUp(singleCmdHelper, self);
}
