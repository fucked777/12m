#include "GTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, KaDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, KaGS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHGMLSK2, createTestItem);

QString GTTest::typeID() { return "1_GT"; }
AutomateTestBase* GTTest::createTestItem(AutomateTest* parent) { return new GTTest(parent); }

GTTest::GTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
GTTest::~GTTest() {}
bool GTTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
        return false;
    }
    /* 这个频率要变成装订的频率 */
    m_downFreq = m_testPlanItem.automateTestParameterBind.testBindParameterMap["F0"].rawValue.toDouble();
    return true;
}
bool GTTest::linkConfiguration()
{
    /* 切换链路
     * G/T值测试
     * S
     * 校零变频器--->LNA--->开关网络--->S频段接收矩阵--->信号选择开关--->后端频谱仪
     * ka遥测/Ka低速
     * 校零变频器--->LNA--->开关网络--->高频箱--->S频段接收矩阵--->信号选择开关--->后端频谱仪
     * Ka高速
     * 校零变频器--->LNA--->开关网络--->高频箱--->信号选择开关--->后端频谱仪
     *
     * 这里只是复用链路切换的代码
     * G/T是连频率都要更换的
     */
    if (!setConfigMacroAndParamMacro(LinkType::RWFS))
    {
        return false;
    }
    /* 将测试信号接入测试计算机--后端 */
    auto result = AutomateTestHelper::backEndSignalToTestComputerDown(*m_singleCommandHelper, this);
    if (!result)
    {
        return false;
    }

    /* 后端测试信号选择开关
     * 在S Ka遥测 Ka低速时需要把 开关打到S入1
     * 在高速时需要打到1.2G入
     */
    result = AutomateTestHelper::backEndSwitchChangeDown(*m_singleCommandHelper, this);
    if (!result)
    {
        return false;
    }

    return true;
}
bool GTTest::parameterSetting()
{
    /* 参数设置
     * 这里要设置的有
     * 1.改变校零变频器输出频率为参数装订的F0  改变衰减为参数装订的A  切换为信标模式
     * 2.改变高频箱的工作频率为F0 衰减
     */
    PackCommand packCommand;

    /* 改变校零变频器输出频率为参数装订的F0  改变衰减为参数装订的A  切换为信标模式 开电 */
    auto& bindMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
    QVariantMap replaceParamMap;
    replaceParamMap["BeaconOutSigFreq"] = m_downFreq; /* 最上方已经改成了装订的频率 */
    replaceParamMap["BeaconAtten"] = bindMap["XLAttenuation"].rawValue;

    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        m_singleCommandHelper->packSingleCommand("StepSXB_START", packCommand, replaceParamMap);
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
    {
        m_singleCommandHelper->packSingleCommand("StepKaXB_START", packCommand, replaceParamMap);
    }

    if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    {
        return false;
    }

    replaceParamMap.clear();
    replaceParamMap["SBandDownFreq"] = bindMap["F0"].rawValue;
    replaceParamMap["RadioFrequDecrement_A"] = bindMap["GPXAttenuation"].rawValue;

    /* 改变高频箱的工作频率为F0 S不需要变频所以不用改 */
    if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        m_singleCommandHelper->packSingleCommand((isLeftSpin() ? "Step_kackgzgpx_kaFreqAndgain_5" : "Step_kackgzgpx_kaFreqAndgain_6"), packCommand,
                                                 replaceParamMap);
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
    {
        m_singleCommandHelper->packSingleCommand((isLeftSpin() ? "Step_KaDSTrack_gain_5" : "Step_KaDSTrack_gain_6"), packCommand, replaceParamMap);
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    {
        m_singleCommandHelper->packSingleCommand((isLeftSpin() ? "Step_KaGSSCGPX_freq_gain_3" : "Step_KaGSSCGPX_freq_gain_4"), packCommand,
                                                 replaceParamMap);
    }

    if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    {
        return false;
    }
    return true;
}
bool GTTest::testing(CmdResult& cmdResult)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        replaceParamMap["SpectCenter"] = m_downFreq; /* 中心频率 */
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    {
        replaceParamMap["SpectCenter"] = 1200 * 1000;
    }
    else
    {
        replaceParamMap["SpectCenter"] = 2250 * 1000;
    }

    replaceParamMap["SpectSelect"] = 1;  /* 频谱仪1后端2前端 */
    replaceParamMap["OutputSwitch"] = 1; /* 信号源关 0on 1off */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    auto stepKey = (m_testPlanItem.workMode == SystemWorkMode::KaGS ? "Step_TC_SBF" : "Step_TC_SBF_1_2G");
    m_singleCommandHelper->packSingleCommand(stepKey, packCommand, replaceParamMap);
    return (waitExecResponceData(cmdResult, packCommand, 3, 40) == ExecStatus::Success);
}
bool GTTest::resultOperation(const CmdResult& cmdResult)
{
    auto curTestResultfind = cmdResult.m_paramdataMap.find("S/φRealValu");
    if (curTestResultfind == cmdResult.m_paramdataMap.end())
    {
        pushLog("测试错误:未能获取当前的S/φ值", true);
        return false;
    }
    auto testResult = curTestResultfind->toDouble();
    /* 拿到S/φ的结果了 */
    /* G/T = G/T0 +（S/Φx - S/Φ0）
     * 就是G/t值等于 人工标校的G/T0加上 当前测试的S/Φx与人工标校S/Φ之差
     */
    auto& bindValueMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
    auto bindFind = bindValueMap.find("SBF");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订S/Φ0参数错误", true);
        return false;
    }
    auto sbf0 = bindFind->rawValue.toDouble();

    bindFind = bindValueMap.find("GT0");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订GT0参数错误", true);
        return false;
    }
    auto gt0 = bindFind->rawValue.toDouble();

    auto desGT = gt0 + (testResult - sbf0);
    // S/φRealValu  S/φ实测值
    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "G/T值";
    automateTestItemParameterInfo.displayValue = QString::number(desGT, 'f', 3);
    automateTestItemParameterInfo.id = "G/T";
    automateTestItemParameterInfo.rawValue = desGT;
    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "G/T值";
    automateTestResultGroup.resultList << automateTestItemParameterInfo;
    automateTestResult.testResultList << automateTestResultGroup;

    automateTestResult.testTime = GlobalData::currentDateTime();

    /* 保存数据 */
    auto msg = AutomateTestHelper::toLogString(automateTestResultGroup);
    msg += QString("\nS/φ:%1").arg(testResult, 0, 'f', 3);
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
void GTTest::endOfTest()
{
    PackCommand packCommand;
    /* 校零变频器关电 */
    AutomateTestHelper::xlbpqPowerOff(*m_singleCommandHelper, this);

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    auto stepKey = (m_testPlanItem.workMode == SystemWorkMode::KaGS ? "Step_TC_SBF" : "Step_TC_SBF_1_2G");
    m_singleCommandHelper->packSingleCommand(stepKey, packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
