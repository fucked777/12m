#include "LBTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SKT, createTestItem);

QString LBTTest::typeID() { return "27_LBT"; }
AutomateTestBase* LBTTest::createTestItem(AutomateTest* parent) { return new LBTTest(parent); }

LBTTest::LBTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
LBTTest::~LBTTest() {}
void LBTTest::startImpl()
{
    pushLog("环路带宽测试开始");
    pushStatus(AutomateTestStatus::ParameterSetting);
    /* 没写现在只写了最终的测试步骤 wp?? */
    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // TODO 配置中频闭环
    LinkLine linkLine;
    linkLine.workMode = m_testPlanItem.workMode;
    linkLine.masterTargetNo = 1;
    TargetInfo targetInfo;
    targetInfo.targetNo = 1;
    targetInfo.workMode = m_testPlanItem.workMode;
    targetInfo.taskCode = m_testPlanItem.taskCode;
    targetInfo.pointFreqNo = m_testPlanItem.dotDrequency;
    linkLine.targetMap[1] = targetInfo;

    linkLine.linkType = LinkType::ZPBH;

    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;
    msg.appendLine(linkLine);

    ControlFlowHandler controlFlowHandler;
    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &AutomateTestBase::signalSendToDevice);
    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &AutomateTestBase::signalInfoMsg);
    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &AutomateTestBase::signalWarningMsg);
    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &AutomateTestBase::signalErrorMsg);

    controlFlowHandler.handle(msg);

    //共载波和不共载波 高码率+扩2 暂时没做
    if (this->m_testPlanItem.workMode == SystemWorkMode::STTC)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_LBT_1", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::Skuo2)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_LBT_4", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::SYTHSMJ)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_LBT_10", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::SYTHWX)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_LBT_11", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::SYTHGML)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_LBT_12", packCommand);
    }

    if (this->m_testPlanItem.workMode == SystemWorkMode::SKT)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_LBT_21", packCommand);
    }

    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 80);
    int index = 1;
    for (auto item : cmdResultList)
    {
        if (item.m_paramdataMap.contains("Bandwidth5") && item.m_paramdataMap.contains("Bandwidth10") &&
            item.m_paramdataMap.contains("Bandwidth100") && item.m_paramdataMap.contains("Bandwidth500") &&
            item.m_paramdataMap.contains("Bandwidth1000") && item.m_paramdataMap.contains("Bandwidth2000"))
        {
            auto testStates = item.m_paramdataMap.value("TestStatus").toInt();
            auto testState = item.m_paramdataMap.value("TestState").toInt();
            //如果是测试完成就推送结果
            if (testState == 2 || testStates == 2 || testState == 1 || testStates == 1)
            {
                auto Bandwidth5 = item.m_paramdataMap.value("Bandwidth5").toDouble();
                auto Bandwidth10 = item.m_paramdataMap.value("Bandwidth10").toDouble();
                auto Bandwidth100 = item.m_paramdataMap.value("Bandwidth100").toDouble();
                auto Bandwidth500 = item.m_paramdataMap.value("Bandwidth500").toDouble();
                auto Bandwidth1000 = item.m_paramdataMap.value("Bandwidth1000").toDouble();
                auto Bandwidth2000 = item.m_paramdataMap.value("Bandwidth2000").toDouble();

                pushLog(QString("带宽5Hz测试值%1Hz").arg(Bandwidth5));
                pushLog(QString("带宽10Hz测试值%1Hz").arg(Bandwidth10));
                pushLog(QString("带宽100Hz测试值%1Hz").arg(Bandwidth100));
                pushLog(QString("带宽500Hz测试值%1Hz").arg(Bandwidth500));
                pushLog(QString("带宽1000Hz测试值%1Hz").arg(Bandwidth1000));
                pushLog(QString("带宽2000Hz测试值%1Hz").arg(Bandwidth2000));
                realCmdResultList.append(Bandwidth5);
                realCmdResultList.append(Bandwidth10);
                realCmdResultList.append(Bandwidth100);
                realCmdResultList.append(Bandwidth500);
                realCmdResultList.append(Bandwidth1000);
                realCmdResultList.append(Bandwidth2000);
            }
        }
        else
        {
            pushLog("配置文件解析失败，无法显示数据");
        }
    }
    pushLog("等效环路带宽测试完成");
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

        if (index == 1)
            automateTestItemParameterInfo.displayName = QString("带宽5Hz测试值");
        if (index == 2)
            automateTestItemParameterInfo.displayName = QString("带宽10Hz测试值");
        if (index == 3)
            automateTestItemParameterInfo.displayName = QString("带宽100Hz测试值");
        if (index == 4)
            automateTestItemParameterInfo.displayName = QString("带宽500Hz测试值");
        if (index == 5)
            automateTestItemParameterInfo.displayName = QString("带宽1000Hz测试值");
        if (index == 6)
            automateTestItemParameterInfo.displayName = QString("带宽2000Hz测试值");
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "TFTT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = QString("等效环路带宽 ").arg(qPow(10, index % 7) / 10);
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
        return;
    }

    pushLog("数据保存成功");

    pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
}
