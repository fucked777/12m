#include "STTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(STTTest, XGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(STTTest, KaGS, createTestItem);

QString STTTest::typeID() { return "20_STT"; }
AutomateTestBase* STTTest::createTestItem(AutomateTest* parent) { return new STTTest(parent); }

STTTest::STTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
STTTest::~STTTest() {}
void STTTest::startImpl()
{
    pushLog("杂散测试开始");
    pushStatus(AutomateTestStatus::ParameterSetting);
    /* 没写现在只写了最终的测试步骤 wp?? */
    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    //打通信号环路
    openLoop();

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;             /* 开始测试 */
    replaceParamMap["SpectCenter"] = m_downFreq; /* 中心频率 */
    replaceParamMap["SpectSelect"] = 1;          /* 频谱仪  1:后端  2:前端 */
    replaceParamMap["SignalFreq"] = m_downFreq;  /* 信号频率 */

    /* key对应客户端的配置文件 */
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }

    singleCmdHelper.packSingleCommand("Step_TC_XXLLZSCS", packCommand, replaceParamMap);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 40);
    for (auto item : cmdResultList)
    {
        if (item.m_paramdataMap.contains("SpurValue"))
        {
            auto val = item.m_paramdataMap.value("SpurValue").toDouble();

            pushLog(QString("杂散值：%1dBc").arg(val));
            realCmdResultList.append(val);
        }
        else
        {
            pushLog("配置文件解析失败，无法显示数据");
        }
    }

    if (execStatus != ExecStatus::Success)
    {
        return;
    }
    pushLog("杂散测试结束");

    // 结果入库
    AutomateTestResult automateTestResult;
    //    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "杂散测试";
    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "杂散值(dBc)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "STT";
        automateTestItemParameterInfo.rawValue = item;

        automateTestResultGroup.resultList << automateTestItemParameterInfo;
    }
    automateTestResult.testResultList << automateTestResultGroup;

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

void STTTest::stopImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    /* 停止当前测试 */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand("Step_TC_XXLLZSCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}

void STTTest::openLoop()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // TODO 配置射频模拟源闭环
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
    //信号源接入后端测试开关网络
    singleCmdHelper.packSingleCommand("Step_TC_XHYTOHZCSKGWL", packCommand);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }

    // 1.2G中频开关矩阵输出口7选择下变频器
    QVariantMap replaceParamMap;
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }
    singleCmdHelper.packSingleCommand("Step_TC_ZPKGJZXZXBPQX", packCommand, replaceParamMap);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }

    //后端测试开关网络接入频谱仪  1.2GHz监测信号
    singleCmdHelper.packSingleCommand("Step_TC_HZCSKGWLTOPPY2", packCommand);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
}
