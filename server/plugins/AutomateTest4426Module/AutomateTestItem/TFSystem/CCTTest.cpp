#include "CCTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, KaDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, KaGS, createTestItem);
// 次测试无X
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, XDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(CCTTest, SYTHGMLSK2, createTestItem);

QString CCTTest::typeID() { return "13_CCT"; }
AutomateTestBase* CCTTest::createTestItem(AutomateTest* parent) { return new CCTTest(parent); }

CCTTest::CCTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
CCTTest::~CCTTest() {}

bool CCTTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
        return false;
    }

    auto getOnlineACUResult = AutomateTestHelper::getOnlineACU();
    if (!getOnlineACUResult)
    {
        pushLog(QString("测试错误:%1").arg(getOnlineACUResult.msg()), true);
        return false;
    }
    m_deviceACUMaster = getOnlineACUResult.value();
    return true;
}
bool CCTTest::linkConfiguration() { return true; }
bool CCTTest::parameterSetting() { return true; }
bool CCTTest::testing(CmdResult& cmdResult)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    /*
     * 1S
     * 2X 26没有X跟踪所以无效
     * 3Ka
     */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        replaceParamMap["TestFreq"] = 1; /* 测试频段 */
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
    {
        replaceParamMap["TestFreq"] = 3; /* 测试频段 */
    }
    else
    {
        pushLog(QString("测试错误:当前频段不正确"), true);
        return false;
    }

    /*
     * 1 开始测试
     * 2 正在测试
     * 3 测试完成
     * 4 结束测试
     * 5 异常终止
     * 6 不具备状态
     */
    m_singleCommandHelper->packSingleCommand("Step_ACU_AUTOTESTCCT", packCommand, m_deviceACUMaster, replaceParamMap);
    auto execStatus = waitExecResponceCondition(cmdResult, packCommand, 3, 1200, {}, [](CmdResult& cmdResult, QString& errMsg, const QVariant&) {
        if (!cmdResult.m_paramdataMap.contains("TestStatus"))
        {
            errMsg = "当前测试数据无测试状态";
            return ConditionResult::Error;
        }
        auto status = cmdResult.m_paramdataMap.value("TestStatus").toInt();
        if (status == 1 || status == 2)
        {
            return ConditionResult::Wait;
        }
        if (status == 3 || status == 4)
        {
            return ConditionResult::OK;
        }
        if (status == 5)
        {
            errMsg = "测试异常终止";
            return ConditionResult::Error;
        }
        if (status == 6)
        {
            errMsg = "测试不具备状态";
            return ConditionResult::Error;
        }
        errMsg = "未知测试错误";
        return ConditionResult::Error;
    });
    return (execStatus == ExecStatus::Success);
}
bool CCTTest::resultOperation(const CmdResult& cmdResult)
{
    if (!cmdResult.m_paramdataMap.contains("AzMaxVel"))
    {
        pushLog("测试错误:当前上报结果 方位最大超调量 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("PitchMaxVel"))
    {
        pushLog("测试错误:当前上报结果 俯仰最大超调量 数据缺失", true);
        return false;
    }
    auto azMaxVel = cmdResult.m_paramdataMap.value("AzMaxVel");
    auto pitchMaxVel = cmdResult.m_paramdataMap.value("PitchMaxVel");

    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "收敛特性";

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "方位最大超调量";
    automateTestItemParameterInfo.displayValue = azMaxVel.toString();
    automateTestItemParameterInfo.id = "AzMaxVel";
    automateTestItemParameterInfo.rawValue = azMaxVel;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "俯仰最大超调量";
    automateTestItemParameterInfo.displayValue = pitchMaxVel.toString();
    automateTestItemParameterInfo.id = "PitchMaxVel";
    automateTestItemParameterInfo.rawValue = pitchMaxVel;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestResult.testResultList << automateTestResultGroup;

    automateTestResult.testTime = GlobalData::currentDateTime();

    /* 保存数据 */
    auto msg = AutomateTestHelper::toLogString(automateTestResultGroup);
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
    return false;
}

void CCTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_ACU_AUTOTESTCCT", packCommand, m_deviceACUMaster, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
