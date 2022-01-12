#include "TDDTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, KaKuo2, createTestItem);

// 扩跳无此测试
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SKT, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHGMLSK2, createTestItem);

QString TDDTTest::typeID() { return "28_TDDT"; }
AutomateTestBase* TDDTTest::createTestItem(AutomateTest* parent) { return new TDDTTest(parent); }

TDDTTest::TDDTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
TDDTTest::~TDDTTest() {}

bool TDDTTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
        return false;
    }
    if (!m_configMacroData.getCKJDDeviceID(m_testPlanItem.workMode, m_ckjd, 1))
    {
        pushLog("获取当前主用测控基带错误", true);
        return false;
    }

    return true;
}
bool TDDTTest::linkConfiguration()
{ /* 切换链路
   * 中频闭环
   * 基带--->前端(模拟源)--->发射矩阵--->接收矩阵--->前端
   */
    return setConfigMacroAndParamMacro(LinkType::SZHBH);
}
bool TDDTTest::parameterSetting() { return true; }
bool TDDTTest::testing(CmdResult& /*cmdResult*/)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    replaceParamMap["RotatDirect"] = isLeftSpin() ? 1 : 2;
    if (this->m_testPlanItem.workMode == SystemWorkMode::STTC)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TDDT_1", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::Skuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TDDT_4", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TDDT_5", packCommand, m_ckjd, replaceParamMap);
    }

    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 30);
    return (execStatus == ExecStatus::Success);
}
bool TDDTTest::resultOperation(const CmdResult& /*cmdResult*/)
{
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("TeleDemDel"))
        {
            auto testStates = item.m_paramdataMap.value("TestStatus").toInt();
            auto testState = item.m_paramdataMap.value("TestState").toInt();
            //如果是测试完成就推送结果
            if (testState == 2 || testStates == 2)
            {
                auto val = item.m_paramdataMap.value("TeleDemDel").toDouble();

                pushLog(QString("遥测解调时延：%1ms").arg(val));
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
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "遥测解调时延(ms)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "TDDT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "遥测解调时延测试";
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

void TDDTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    if (this->m_testPlanItem.workMode == SystemWorkMode::STTC)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TDDT_1", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::Skuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TDDT_4", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TDDT_5", packCommand, m_ckjd, replaceParamMap);
    }
    notWaitExecSuccessBeQuiet(packCommand);
}
