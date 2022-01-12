#include "RPNTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(RPNTTest, XGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RPNTTest, KaGS, createTestItem);

QString RPNTTest::typeID() { return "19_RPNT"; }
AutomateTestBase* RPNTTest::createTestItem(AutomateTest* parent) { return new RPNTTest(parent); }

RPNTTest::RPNTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
RPNTTest::~RPNTTest() {}
void RPNTTest::startImpl()
{
    pushLog("相位噪声测试开始");
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
    replaceParamMap["TestCtrl"] = 1;    /* 开始测试 */
    replaceParamMap["SpectSelect"] = 1; /* 频谱仪  1:后端  2:前端 */
    if (m_testPlanItem.workMode == XGS)
    {
        replaceParamMap["SpectCenter"] = m_downFreq - 1200000; /* 中心频率 */
    }
    else if (m_testPlanItem.workMode == KaGS)
    {
        if (!m_testPlanItem.testParameterMap.contains("24Choose1Switch"))
        {
            pushLog("获取本振数据失败");
            return;
        }
        auto value = m_testPlanItem.testParameterMap["24Choose1Switch"].rawValue.toInt();
        auto result = value % 2;
        if (result == 0)  //偶数  本振2
        {
            replaceParamMap["SpectCenter"] = m_downFreq - 5400000 - 1200000; /* 中心频率 */
        }
        else if (result == 1)  //奇数  本振1
        {
            replaceParamMap["SpectCenter"] = 5400000; /* 中心频率 */
        }
    }

    /* key对应客户端的配置文件 */
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }

    singleCmdHelper.packSingleCommand("Step_TC_XXXWZSCS", packCommand, replaceParamMap);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 140);

    for (auto item : cmdResultList)
    {
        if (item.m_paramdataMap.contains("10Spect") && item.m_paramdataMap.contains("100Spect") && item.m_paramdataMap.contains("1KSpect") &&
            item.m_paramdataMap.contains("10KSpect") && item.m_paramdataMap.contains("100KSpect"))
        {
            auto Spect10 = item.m_paramdataMap.value("10Spect").toDouble();
            auto Spect100 = item.m_paramdataMap.value("100Spect").toDouble();
            auto Spect1K = item.m_paramdataMap.value("1KSpect").toDouble();
            auto Spect10K = item.m_paramdataMap.value("10KSpect").toDouble();
            auto Spect100K = item.m_paramdataMap.value("100KSpect").toDouble();

            pushLog(QString("10Hz相噪：%1 dBc/Hz").arg(Spect10));
            pushLog(QString("100Hz相噪：%1 dBc/Hz").arg(Spect100));
            pushLog(QString("1kHz相噪：%1 dBc/Hz").arg(Spect1K));
            pushLog(QString("10kHz相噪：%1 dBc/Hz").arg(Spect10K));
            pushLog(QString("100kHz相噪：%1dBc/Hz").arg(Spect100K));

            realCmdResultList.append(Spect10);
            realCmdResultList.append(Spect100);
            realCmdResultList.append(Spect1K);
            realCmdResultList.append(Spect10K);
            realCmdResultList.append(Spect100K);
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
    pushLog("相位噪声测试结束");
    // 结果入库
    AutomateTestResult automateTestResult;
    //    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "相位噪声测试结果";
    int index = 1;
    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;

        if (index == 1)
            automateTestItemParameterInfo.displayName = "10Hz相噪(dBc/Hz)";
        if (index == 2)
            automateTestItemParameterInfo.displayName = "100Hz相噪(dBc/Hz)";
        if (index == 3)
            automateTestItemParameterInfo.displayName = "1kHz相噪(dBc/Hz)";
        if (index == 4)
            automateTestItemParameterInfo.displayName = "10kHz相噪(dBc/Hz)";
        if (index == 5)
            automateTestItemParameterInfo.displayName = "100kHz相噪(dBc/Hz)";
        index++;

        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "RPNT";
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

void RPNTTest::stopImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    /* 停止当前测试 */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand("Step_TC_XXXWZSCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}

void RPNTTest::openLoop()
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

    // 24选1开关接入本振信号
    QVariantMap replaceParamMap;
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }
    singleCmdHelper.packSingleCommand("Step_TC_HD24C1BZXH", packCommand, replaceParamMap);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }

    //后端测试开关网络接入频谱仪  主控机房本振及监测信号
    singleCmdHelper.packSingleCommand("Step_TC_HZCSKGWLTOPPY", packCommand);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
}
