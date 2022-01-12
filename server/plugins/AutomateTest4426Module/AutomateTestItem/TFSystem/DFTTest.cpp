#include "DFTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, KaDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, KaGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, XDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(DFTTest, SYTHGMLSK2, createTestItem);

QString DFTTest::typeID() { return "12_DFT"; }
AutomateTestBase* DFTTest::createTestItem(AutomateTest* parent) { return new DFTTest(parent); }

DFTTest::DFTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
DFTTest::~DFTTest() {}
bool DFTTest::pretreatment()
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
bool DFTTest::linkConfiguration() { return true; }
bool DFTTest::parameterSetting() { return true; }
bool DFTTest::testing(CmdResult& cmdResult)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    /* 这个东东好像 无论传方位还是俯仰都是上报两个数据 20210831 wp?? */
    // replaceParamMap["TestAxis"] = testAxis; /* 测试轴 */
    replaceParamMap["TestAxis"] = 1; /* 测试轴 */

    /*
     * 1 开始测试
     * 2 正在测试
     * 3 测试完成
     * 4 结束测试
     * 5 异常终止
     * 6 不具备状态
     */
    m_singleCommandHelper->packSingleCommand("Step_ACU_AUTOTESTDFT", packCommand, m_deviceACUMaster, replaceParamMap);
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
bool DFTTest::resultOperation(const CmdResult& cmdResult)
{
    /* 这个东东好像 无论传方位还是俯仰都是上报两个数据 20210831 wp?? */
    if (!cmdResult.m_paramdataMap.contains("AzMaxVel"))
    {
        pushLog("测试错误:当前上报结果 方位最大速度 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("PitchMaxVel"))
    {
        pushLog("测试错误:当前上报结果 俯仰最大速度 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("AzMaxAcce"))
    {
        pushLog("测试错误:当前上报结果 方位最大加速度 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("PitchMaxAcce"))
    {
        pushLog("测试错误:当前上报结果 俯仰最大加速度 缺失", true);
        return false;
    }

    auto azMaxVel = cmdResult.m_paramdataMap.value("AzMaxVel");
    auto pitchMaxVel = cmdResult.m_paramdataMap.value("PitchMaxVel");
    auto azMaxAcce = cmdResult.m_paramdataMap.value("AzMaxAcce");
    auto pitchMaxAcce = cmdResult.m_paramdataMap.value("PitchMaxAcce");

    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "伺服带宽";

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "方位最大速度";
    automateTestItemParameterInfo.displayValue = azMaxVel.toString();
    automateTestItemParameterInfo.id = "AzMaxVel";
    automateTestItemParameterInfo.rawValue = azMaxVel;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "俯仰最大速度";
    automateTestItemParameterInfo.displayValue = pitchMaxVel.toString();
    automateTestItemParameterInfo.id = "PitchMaxVel";
    automateTestItemParameterInfo.rawValue = pitchMaxVel;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "方位最大加速度";
    automateTestItemParameterInfo.displayValue = azMaxAcce.toString();
    automateTestItemParameterInfo.id = "AzMaxAcce";
    automateTestItemParameterInfo.rawValue = azMaxAcce;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "俯仰最大加速度";
    automateTestItemParameterInfo.displayValue = pitchMaxAcce.toString();
    automateTestItemParameterInfo.id = "PitchMaxAcce";
    automateTestItemParameterInfo.rawValue = pitchMaxAcce;
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
    return true;
}

void DFTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_ACU_AUTOTESTDFT", packCommand, m_deviceACUMaster, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
