#include "SBTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, KaDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, KaGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, XDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SBTTest, SYTHGMLSK2, createTestItem);

QString SBTTest::typeID() { return "11_SBT"; }
AutomateTestBase* SBTTest::createTestItem(AutomateTest* parent) { return new SBTTest(parent); }

SBTTest::SBTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
SBTTest::~SBTTest() {}

bool SBTTest::pretreatment()
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

bool SBTTest::linkConfiguration() { return true; }
bool SBTTest::parameterSetting() { return true; }
bool SBTTest::testing(CmdResult& cmdResult)
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
    m_singleCommandHelper->packSingleCommand("Step_ACU_AUTOTESTSBT", packCommand, m_deviceACUMaster, replaceParamMap);
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
bool SBTTest::resultOperation(const CmdResult& cmdResult)
{ /* 这个东东好像 无论传方位还是俯仰都是上报两个数据 20210831 wp?? */
    if (!cmdResult.m_paramdataMap.contains("AzimBand"))
    {
        pushLog("测试错误:当前上报结果 方位带宽 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("PitchBand"))
    {
        pushLog("测试错误:当前上报结果 俯仰带宽 数据缺失", true);
        return false;
    }
    auto azimBand = cmdResult.m_paramdataMap.value("AzimBand");
    auto pitchBand = cmdResult.m_paramdataMap.value("PitchBand");

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
    automateTestItemParameterInfo.displayName = "方位带宽";
    automateTestItemParameterInfo.displayValue = azimBand.toString();
    automateTestItemParameterInfo.id = "AzimBand";
    automateTestItemParameterInfo.rawValue = azimBand;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "俯仰带宽";
    automateTestItemParameterInfo.displayValue = pitchBand.toString();
    automateTestItemParameterInfo.id = "PitchBand";
    automateTestItemParameterInfo.rawValue = pitchBand;
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

void SBTTest::endOfTest()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand("Step_ACU_AUTOTESTSBT", packCommand, m_deviceACUMaster, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
