#include "TFTTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, KaDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHGMLSK2, createTestItem);

QString TFTTTest::typeID() { return "23_TFTT"; }
AutomateTestBase* TFTTTest::createTestItem(AutomateTest* parent) { return new TFTTTest(parent); }

TFTTTest::TFTTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
TFTTTest::~TFTTTest() {}

bool TFTTTest::pretreatment()
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
bool TFTTTest::linkConfiguration()
{
    /* 切换链路
     * 中频闭环
     * 基带--->前端(模拟源)--->发射矩阵--->接收矩阵--->前端
     * 待定
     */
    return setConfigMacroAndParamMacro(LinkType::SZHBH);
}
bool TFTTTest::parameterSetting()
{
    //  /* 待定 */
    //  /* 切换为前端输出 */
    //  if (!m_configMacroData.getCKQDDeviceID(m_testPlanItem.workMode, m_ckQDDeviceID))
    //  {
    //      pushLog("获取当前测控前端错误", true);
    //      return;
    //  }

    //  QVariantMap replace;
    //  replace["SendSource_2"] = 2;
    //  singleCmdHelper.packSingleCommand("StepCKQD_SignalSource", packCommand, m_ckQDDeviceID, replace);
    //  if (AutomateTestBase::ExecStatus::Success != waitExecSuccess(packCommand, 40))
    //  {
    //      return;
    //  }
    return true;
}
bool TFTTTest::testing(CmdResult& /*cmdResult*/)
{
    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    PackCommand packCommand;
    m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TFTT", packCommand);
    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 30);
    return (execStatus == ExecStatus::Success);
}
bool TFTTTest::resultOperation(const CmdResult& /*cmdResult*/)
{
    int index = 1;
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("AGCTime1") && item.m_paramdataMap.contains("AGCTime10") && item.m_paramdataMap.contains("AGCTime100") &&
            item.m_paramdataMap.contains("AGCTime1000") && item.m_paramdataMap.contains("TestState"))
        {
            auto AGCTime1 = item.m_paramdataMap.value("AGCTime1").toDouble();
            auto AGCTime10 = item.m_paramdataMap.value("AGCTime10").toDouble();
            auto AGCTime100 = item.m_paramdataMap.value("AGCTime100").toDouble();
            auto AGCTime1000 = item.m_paramdataMap.value("AGCTime1000").toDouble();
            auto testState = item.m_paramdataMap.value("TestState").toInt();
            //如果是测试完成就推送结果
            if (testState == 2)
            {
                pushLog(QString("[AGC时间常数]AGC时间常数理论值：1ms AGC时间常数测试值%1：%2ms").arg(index).arg(AGCTime1));
                pushLog(QString("[AGC时间常数]AGC时间常数理论值：10ms AGC时间常数测试值%1：%2ms").arg(index).arg(AGCTime10));
                pushLog(QString("[AGC时间常数]AGC时间常数理论值：100ms AGC时间常数测试值%1：%2ms").arg(index).arg(AGCTime100));
                pushLog(QString("[AGC时间常数]AGC时间常数理论值：1000ms AGC时间常数测试值%1：%2ms").arg(index).arg(AGCTime1000));
                index++;
                realCmdResultList.append(AGCTime1);
                realCmdResultList.append(AGCTime10);
                realCmdResultList.append(AGCTime100);
                realCmdResultList.append(AGCTime1000);
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

    index = 1;
    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = QString("AGC时间常数测试值%1").arg(index / 4 + 1);
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "TFTT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = QString("[AGC时间常数]AGC时间常数理论值：%1ms ").arg(qPow(10, (index - 1) % 4));
        automateTestResultGroup.resultList << automateTestItemParameterInfo;
        automateTestResult.testResultList << automateTestResultGroup;
        index++;
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
void TFTTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_TFTT", packCommand, m_ckQDDeviceID, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
