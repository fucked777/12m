#include "EIRPTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, KaDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(EIRPTest, SYTHGMLSK2, createTestItem);

QString EIRPTest::typeID() { return "2_EIRP"; }
AutomateTestBase* EIRPTest::createTestItem(AutomateTest* parent) { return new EIRPTest(parent); }

EIRPTest::EIRPTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
EIRPTest::~EIRPTest() {}
bool EIRPTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
        return false;
    }

    PackCommand packCommand;

    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        if (!m_configMacroData.getSGGFDeviceID(m_testPlanItem.workMode, m_hpID))
        {
            pushLog("获取当前主用功放错误", true);
            return false;
        }
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        if (!m_configMacroData.getKaGGFDeviceID(m_testPlanItem.workMode, m_hpID))
        {
            pushLog("获取当前主用功放错误", true);
            return false;
        }
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
    {
        if (!m_configMacroData.getKaDtGGFDeviceID(m_testPlanItem.workMode, m_hpID))
        {
            pushLog("获取当前主用功放错误", true);
            return false;
        }
    }

    return true;
}

bool EIRPTest::linkConfiguration()
{
    /* 切换链路
     * EIRP值测试
     * S
     * 基带--->前端--->功放--->校零变频器--->LNA--->S频段射频开关网络--->信号选择开关--->前端频谱仪
     * ka遥测/低速
     * 基带--->前端--->功放--->校零变频器--->LNA--->Ka频段射频开关网络--->信号选择开关--->前端频谱仪
     */

    if (!setConfigMacroAndParamMacro(LinkType::PKXLBPQBH))
    {
        return false;
    }

    /* 将测试信号接入测试计算机--前端 */
    auto result = AutomateTestHelper::frontEndSignalToTestComputer(*m_singleCommandHelper, this);
    if (!result)
    {
        return false;
    }

    /* 前端测试信号选择开关 射频信号接入  */
    result = AutomateTestHelper::frontRadioEndSwitchChange(*m_singleCommandHelper, this);
    if (!result)
    {
        return false;
    }

    return true;
}
bool EIRPTest::parameterSetting()
{
    /* 参数设置
     * 这里要设置的有
     * 1.功放输出功率为预设功率P0
     * 2.改变校零变频器 改变衰减为参数装订的A
     */
    PackCommand packCommand;
    auto& bindMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
    QVariantMap replaceParamMap;
    replaceParamMap["TransmPower"] = bindMap["HPP0"].rawValue;
    /* 功放功率设置 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        m_singleCommandHelper->packSingleCommand("STEP_SHPA_ON_SPECIFIED_POWER", packCommand, m_hpID, replaceParamMap);
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        m_singleCommandHelper->packSingleCommand("StepKaHPA_ON_SPECIFIED_POWER", packCommand, m_hpID, replaceParamMap);
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
    {
        m_singleCommandHelper->packSingleCommand("StepKaSCHPA_ON_SPECIFIED_POWER", packCommand, m_hpID, replaceParamMap);
    }
    if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    {
        return false;
    }

    /* 改变校零变频器 改变衰减为参数装订的A */
    replaceParamMap.clear();
    replaceParamMap["CheckZeroAtten"] = bindMap["XLAttenuation"].rawValue;

    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        m_singleCommandHelper->packSingleCommand("StepSXL_START", packCommand, replaceParamMap);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
    {
        m_singleCommandHelper->packSingleCommand("StepKaXL_START", packCommand, replaceParamMap);
    }

    if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    {
        return false;
    }

    return true;
}
bool EIRPTest::testing(CmdResult& cmdResult)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        replaceParamMap["SpectCenter"] = m_upFreq; /* 中心频率 */
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    {
        replaceParamMap["SpectCenter"] = 1200 * 1000;
    }
    else
    {
        replaceParamMap["SpectCenter"] = 2250 * 1000;
    }

    replaceParamMap["SpectSelect"] = 2;  /* 频谱仪1后端2前端 */
    replaceParamMap["OutputSwitch"] = 1; /* 信号源关 0on 1off */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    m_singleCommandHelper->packSingleCommand("Step_TC_WZPLHXHDPCS", packCommand, replaceParamMap);
    if (waitExecResponceData(cmdResult, packCommand, 3, 40) != ExecStatus::Success)
    {
        return false;
    }

    return true;
}
bool EIRPTest::resultOperation(const CmdResult& cmdResult)
{
    auto curTestResultfind = cmdResult.m_paramdataMap.find("TranPower");
    if (curTestResultfind == cmdResult.m_paramdataMap.end())
    {
        pushLog("测试错误:未能获取当前的发射功率值", true);
        return false;
    }
    auto testResult = curTestResultfind->toDouble();
    /* 拿到工作频率和信号电平测试的结果了 */
    /* EIRP =EIRP0 -（Px - P0）；
     * 就是EIRP值等于 人工标校的EIRP0减去 当前测试的Px与人工标校P0之差
     */
    auto& bindValueMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
    auto bindFind = bindValueMap.find("P0");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订P0参数错误", true);
        return false;
    }
    auto p0 = bindFind->rawValue.toDouble();

    bindFind = bindValueMap.find("EIRP0");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订EIRP0参数错误", true);
        return false;
    }
    auto eirp0 = bindFind->rawValue.toDouble();

    auto desEIRP = eirp0 - (testResult - p0);

    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "EIRP值";
    automateTestItemParameterInfo.displayValue = QString::number(desEIRP, 'f', 3);
    automateTestItemParameterInfo.id = "EIRP";
    automateTestItemParameterInfo.rawValue = desEIRP;
    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "EIRP值";
    automateTestResultGroup.resultList << automateTestItemParameterInfo;
    automateTestResult.testResultList << automateTestResultGroup;

    automateTestResult.testTime = GlobalData::currentDateTime();

    /* 保存数据 */
    auto msg = AutomateTestHelper::toLogString(automateTestResultGroup);
    msg += QString("\nEIRP:%1").arg(testResult, 0, 'f', 3);
    auto result = SqlAutomateTest::insertTestResultItem(automateTestResult);
    if (!result)
    {
        msg = QString("%1\n数据保存失败:%2").arg(msg, result.msg());
        pushLog(msg, true);
        return false;
    }

    pushLog(msg);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
    return true;
}

void EIRPTest::endOfTest()
{
    PackCommand packCommand;

    /* 校零变频器关电 */
    AutomateTestHelper::xlbpqPowerOff(*m_singleCommandHelper, this);
    /* 功放关电 */
    AutomateTestHelper::hpPowerOff(*m_singleCommandHelper, this, m_hpID);

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_TC_WZPLHXHDPCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}

#if 0
/* 走的S接收矩阵 */
void EIRPTest::startImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    pushLog("EIRP值测试开始");

    pushStatus(AutomateTestStatus::LinkConfiguration);
    /* 旋向 配置宏 */
    auto scResult = getSpinAndConfigMacroData(m_testPlanItem.taskCode, m_testPlanItem.workMode, m_testPlanItem.dotDrequency);
    if (!scResult)
    {
        pushLog(scResult.msg(), true);
        return;
    }
    auto& spin = std::get<0>(scResult.value()); /* 1左旋 2 右旋 */
    auto& configMacroData = std::get<1>(scResult.value());

    /* 切换链路
     * EIRP值测试
     * S
     * 基带--->前端--->功放--->校零变频器--->LNA--->S频段接收矩阵--->信号选择开关--->后端频谱仪
     * ka遥测
     * 基带--->前端--->功放--->校零变频器--->LNA--->高频箱--->S频段接收矩阵--->信号选择开关--->后端频谱仪
     */
    {
        ManualMessage msg;
        msg.manualType = ManualType::ConfigMacroAndParamMacro;
        msg.isManualContrl = true;
        msg.configMacroData = configMacroData;

        LinkLine linkLine;
        auto index = 1;
        linkLine.workMode = m_testPlanItem.workMode;
        linkLine.masterTargetNo = index;
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = m_testPlanItem.workMode;
        targetInfo.taskCode = m_testPlanItem.taskCode;
        targetInfo.pointFreqNo = m_testPlanItem.dotDrequency;
        linkLine.targetMap[index] = targetInfo;
        linkLine.linkType = LinkType::PKXLBPQBH;
        msg.appendLine(linkLine);

        ControlFlowHandler controlFlowHandler;
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &EIRPTest::sendByteArrayToDevice);
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, [=](const QString& msg) {
            this->pushLog(msg, true);
            emit this->sg_end();
        });
        controlFlowHandler.handle(msg);

        /* 将测试信号接入测试计算机 */
        {
            auto result = AutomateTestHelper::backEndSignalToTestComputer(singleCmdHelper, m_testPlanItem.workMode, spin, this);
            if (!result)
            {
                return;
            }
        }

        /* 后端测试信号选择开关
         * 在S Ka遥测 Ka低速时需要把 开关打到S入1
         * 在高速时需要打到1.2G入
         */
        {
            auto result = AutomateTestHelper::backEndSwitchChange(singleCmdHelper, m_testPlanItem.workMode, this);
            if (!result)
            {
                return;
            }
        }
    }

    pushStatus(AutomateTestStatus::ParameterSetting);
    /* 参数设置
     * 这里要设置的有
     * 1.功放输出功率为预设功率P0
     * 2.改变校零变频器 改变衰减为参数装订的A
     * 3.改变高频箱的衰减
     */
    {
        auto& bindMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
        QVariantMap replaceParamMap;
        replaceParamMap["TransmPower"] = bindMap["HPP0"].rawValue;
        /* 功放功率设置 */
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
        {
            if (!configMacroData.getSGGFDeviceID(m_testPlanItem.workMode, m_hpID))
            {
                pushLog("获取当前主用功放错误", true);
                return;
            }
            singleCmdHelper.packSingleCommand("STEP_SHPA_ON_SPECIFIED_POWER", packCommand, m_hpID, replaceParamMap);
        }
        else if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
        {
            if (!configMacroData.getKaGGFDeviceID(m_testPlanItem.workMode, m_hpID))
            {
                pushLog("获取当前主用功放错误", true);
                return;
            }
            singleCmdHelper.packSingleCommand("StepKaHPA_ON_SPECIFIED_POWER", packCommand, m_hpID, replaceParamMap);
        }
        else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
        {
            if (!configMacroData.getKaDtGGFDeviceID(m_testPlanItem.workMode, m_hpID))
            {
                pushLog("获取当前主用功放错误", true);
                return;
            }
            singleCmdHelper.packSingleCommand("StepKaSCHPA_ON_SPECIFIED_POWER", packCommand, m_hpID, replaceParamMap);
        }
        auto execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }

        /* 改变校零变频器 改变衰减为参数装订的A */
        replaceParamMap.clear();
        replaceParamMap["CheckZeroAtten"] = bindMap["XLAttenuation"].rawValue;

        if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
        {
            singleCmdHelper.packSingleCommand("StepSXL_START", packCommand, replaceParamMap);
        }
        else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
        {
            singleCmdHelper.packSingleCommand("StepKaXL_START", packCommand, replaceParamMap);
        }

        execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }

        replaceParamMap.clear();
        replaceParamMap["RadioFrequDecrement_A"] = bindMap["GPXAttenuation"].rawValue;

        /* 改变变频器的衰减 S不需要变频所以不用改 */
        if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
        {
            singleCmdHelper.packSingleCommand((isLeftSpin() ? "Step_kackgzgpx_kaFreqAndgain_5" : "Step_kackgzgpx_kaFreqAndgain_6"), packCommand,
                                              replaceParamMap);
        }
        else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
        {
            singleCmdHelper.packSingleCommand((isLeftSpin() ? "Step_KaDSTrack_gain_5" : "Step_KaDSTrack_gain_6"), packCommand, replaceParamMap);
        }

        execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }
    }

    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        replaceParamMap["SpectCenter"] = m_testPlanItem.downFreq; /* 中心频率 */
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    {
        replaceParamMap["SpectCenter"] = 1200 * 1000;
    }
    else
    {
        replaceParamMap["SpectCenter"] = 2250 * 1000;
    }

    replaceParamMap["SpectSelect"] = 2;  /* 频谱仪1后端2前端 */
    replaceParamMap["OutputSwitch"] = 1; /* 信号源关 0on 1off */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    singleCmdHelper.packSingleCommand("Step_TC_WZPLHXHDPCS", packCommand, replaceParamMap);
    CmdResult cmdResult;
    auto execStatus = waitExecResponceData(cmdResult, packCommand, 3, 40);
    if (execStatus != ExecStatus::Success)
    {
        return;
    }

    auto curTestResultfind = cmdResult.m_paramdataMap.find("TranPower");
    if (curTestResultfind == cmdResult.m_paramdataMap.end())
    {
        pushLog("测试错误:未能获取当前的发射功率值", true);
        return;
    }
    auto testResult = curTestResultfind->toDouble();
    /* 拿到工作频率和信号电平测试的结果了 */
    /* EIRP =EIRP0 -（Px - P0）；
     * 就是EIRP值等于 人工标校的EIRP0减去 当前测试的Px与人工标校P0之差
     */
    auto& bindValueMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
    auto bindFind = bindValueMap.find("P0");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订P0参数错误", true);
        return;
    }
    auto p0 = bindFind->rawValue.toDouble();

    bindFind = bindValueMap.find("EIRP0");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订EIRP0参数错误", true);
        return;
    }
    auto eirp0 = bindFind->rawValue.toDouble();

    auto desEIRP = eirp0 - (testResult - p0);

    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "EIRP值";
    automateTestItemParameterInfo.displayValue = QString::number(desEIRP, 'f', 3);
    automateTestItemParameterInfo.id = "EIRP";
    automateTestItemParameterInfo.rawValue = desEIRP;
    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "EIRP值";
    automateTestResultGroup.resultList << automateTestItemParameterInfo;
    automateTestResult.testResultList << automateTestResultGroup;

    automateTestResult.testTime = GlobalData::currentDateTime();

    /* 保存数据 */
    auto msg = QString("当前测得\nEIRP:%1\nG/T:%2").arg(testResult, 0, 'f', 3).arg(desEIRP, 0, 'f', 3);
    auto result = SqlAutomateTest::insertTestResultItem(automateTestResult);
    if (!result)
    {
        msg = QString("%1\n数据保存失败:%2").arg(msg, result.msg());
        pushLog(msg, true);
        return;
    }

    pushLog(msg);
    pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
}

#endif
