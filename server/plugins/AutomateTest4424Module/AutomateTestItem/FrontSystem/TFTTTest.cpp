#include "TFTTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TFTTTest, SKT, createTestItem);

QString TFTTTest::typeID() { return "23_TFTT"; }
AutomateTestBase* TFTTTest::createTestItem(AutomateTest* parent) { return new TFTTTest(parent); }

TFTTTest::TFTTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
TFTTTest::~TFTTTest() {}
void TFTTTest::startImpl()
{
    pushLog("AGC时常测试开始");
    pushStatus(AutomateTestStatus::ParameterSetting);
    /* 没写现在只写了最终的测试步骤 wp?? */
    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // TODO 配置射频模拟源有线闭环
    LinkLine linkLine;
    linkLine.workMode = m_testPlanItem.workMode;
    linkLine.masterTargetNo = 1;
    TargetInfo targetInfo;
    targetInfo.targetNo = 1;
    targetInfo.workMode = m_testPlanItem.workMode;
    targetInfo.taskCode = m_testPlanItem.taskCode;
    targetInfo.pointFreqNo = m_testPlanItem.dotDrequency;
    linkLine.targetMap[1] = targetInfo;

    linkLine.linkType = LinkType::SPMNYYXBH;

    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;
    msg.appendLine(linkLine);

    ControlFlowHandler controlFlowHandler;
    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &AutomateTestBase::signalSendToDevice);
    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &AutomateTestBase::signalInfoMsg);
    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &AutomateTestBase::signalWarningMsg);
    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &AutomateTestBase::signalErrorMsg);

    controlFlowHandler.handle(msg);

    auto configMacroData = controlFlowHandler.getNewConfigMacroData();

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    singleCmdHelper.packSingleCommand("Step_AUTOTEST_TFTT", packCommand);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 30);
    int index = 1;
    for (auto item : cmdResultList)
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
        }
    }
    pushLog("AGC时间常数测试完成");
    if (execStatus != ExecStatus::Success)
    {
        return;
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
        return;
    }

    pushLog("数据保存成功");

    pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
}
