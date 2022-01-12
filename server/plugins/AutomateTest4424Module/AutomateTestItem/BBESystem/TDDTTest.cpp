#include "TDDTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(TDDTTest, SKT, createTestItem);

QString TDDTTest::typeID() { return "28_TDDT"; }
AutomateTestBase* TDDTTest::createTestItem(AutomateTest* parent) { return new TDDTTest(parent); }

TDDTTest::TDDTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
TDDTTest::~TDDTTest() {}
void TDDTTest::startImpl()
{
    pushLog("遥测解调时延测试开始");
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
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_TDDT_1", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::Skuo2)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_TDDT_4", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::SYTHSMJ)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_TDDT_10", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::SYTHWX)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_TDDT_11", packCommand);
    }
    if (this->m_testPlanItem.workMode == SystemWorkMode::SYTHGML)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_TDDT_12", packCommand);
    }

    if (this->m_testPlanItem.workMode == SystemWorkMode::SKT)
    {
        singleCmdHelper.packSingleCommand("Step_AUTOTEST_TDDT_21", packCommand);
    }
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 30);

    for (auto item : cmdResultList)
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
        return;
    }

    pushLog("数据保存成功");
    pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
}
