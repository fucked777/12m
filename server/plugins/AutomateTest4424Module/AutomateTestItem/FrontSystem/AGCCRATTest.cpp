#include "AGCCRATTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(AGCCRATTest, SKT, createTestItem);

QString AGCCRATTest::typeID() { return "22_AGCCRAT"; }
AutomateTestBase* AGCCRATTest::createTestItem(AutomateTest* parent) { return new AGCCRATTest(parent); }

AGCCRATTest::AGCCRATTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
AGCCRATTest::~AGCCRATTest() {}
void AGCCRATTest::startImpl()
{
    pushLog("AGC控制范围、精度测试开始");
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
    //获取测控前端的设备ID
    DeviceID ckQDDeviceID;
    configMacroData.getCKQDDeviceID(m_testPlanItem.workMode, ckQDDeviceID);

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    singleCmdHelper.packSingleCommand("Step_AUTOTEST_AGCCRAT", packCommand, ckQDDeviceID);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 30);
    int index = 1;
    for (auto item : cmdResultList)
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
        }
    }
    pushLog("测试完成AGC控制范围、控制精度测试");
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
        return;
    }

    pushLog("数据保存成功");
    pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
}
