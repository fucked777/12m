#include "AGCCRATTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, KaDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHGMLSK2, createTestItem);

QString AGCCRATTest::typeID() { return "22_AGCCRAT"; }
AutomateTestBase* AGCCRATTest::createTestItem(AutomateTest* parent) { return new AGCCRATTest(parent); }

AGCCRATTest::AGCCRATTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
AGCCRATTest::~AGCCRATTest() {}
bool AGCCRATTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
        return false;
    }
    /* 切换为前端输出 */
    if (!m_configMacroData.getCKQDDeviceID(m_testPlanItem.workMode, m_ckQDDeviceID))
    {
        pushLog("获取当前测控前端错误", true);
        return false;
    }

    return true;
}
bool AGCCRATTest::linkConfiguration()
{
    /* 切换链路
     * 中频闭环
     * 前端(模拟源)--->发射矩阵--->接收矩阵--->前端
     */
    return setConfigMacroAndParamMacro(LinkType::SZHBH);
}
bool AGCCRATTest::parameterSetting()
{
    PackCommand packCommand;
    QVariantMap replace;
    replace["SendSource_2"] = 2;
    m_singleCommandHelper->packSingleCommand("StepCKQD_SignalSource", packCommand, m_ckQDDeviceID, replace);
    if (AutomateTestBase::ExecStatus::Success != waitExecSuccess(packCommand, 40))
    {
        return false;
    }

    return true;
}
bool AGCCRATTest::testing(CmdResult& cmdResult)
{
    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    PackCommand packCommand;
    m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_AGCCRAT", packCommand, m_ckQDDeviceID);

    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 30);
    return (execStatus == ExecStatus::Success);
}
bool AGCCRATTest::resultOperation(const CmdResult& cmdResult)
{
    int index = 1;
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("AGCLevel") && item.m_paramdataMap.contains("TestState"))
        {
            auto val = item.m_paramdataMap.value("AGCLevel").toDouble();
            auto testState = item.m_paramdataMap.value("TestState").toInt();
            //如果是测试完成就推送结果
            if (testState == 2)
            {
                pushLog(QString("[AGC控制范围、控制精度]AGC控制范围:-10dBm~-70dBm;AGC控制精度%1：%2dB").arg(index++).arg(val));
                realCmdResultList.append(val);
            }
        }
        else
        {
            pushLog("配置文件解析失败，无法显示数据");
            return false;
        }
    }

    // 结果入库
    AutomateTestResult automateTestResult;
    //    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "AGC值：";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "AGCCRAT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "[AGC控制范围、控制精度]AGC控制范围:-10dBm~-70dBm;AGC控制精度";
        automateTestResultGroup.resultList << automateTestItemParameterInfo;
        automateTestResult.testResultList << automateTestResultGroup;
    }

    automateTestResult.testTime = GlobalData::currentDateTime();
    /* 保存数据 */

    auto result = SqlAutomateTest::insertTestResultItem(automateTestResult);
    if (!result)
    {
        auto msg = QString("数据保存失败:%1").arg(result.msg());
        pushLog(msg, true);
        return false;
    }

    pushLog("数据保存成功");

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
    return true;
}

void AGCCRATTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_AGCCRAT", packCommand, m_ckQDDeviceID, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
