#include "PSTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "BusinessMacroCommon.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SKT, createTestItem);

QString PSTTest::typeID() { return "15_PST"; }
AutomateTestBase* PSTTest::createTestItem(AutomateTest* parent) { return new PSTTest(parent); }

PSTTest::PSTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
PSTTest::~PSTTest() {}
void PSTTest::startImpl()
{
    pushLog("功率稳定性测试开始");
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
    replaceParamMap["TestCtrl"] = 1;           /* 开始测试 */
    replaceParamMap["SpectCenter"] = m_upFreq; /* 中心频率 */
    replaceParamMap["SpectSelect"] = 2;        /* 频谱仪  1:后端  2:前端 */

    /* key对应客户端的配置文件 */
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }

    //必须要有定时间隔和测试次数
    if (!replaceParamMap.contains("TimeInter") || !replaceParamMap.contains("TestNumber"))
    {
        pushLog("配置文件解析失败，无法测试");
    }
    auto TimeInter = replaceParamMap.value("TimeInter").toInt();
    auto TestNumber = replaceParamMap.value("TestNumber").toInt();
    int waitTime = (TimeInter - 1) * 60 * TestNumber + 40 * TestNumber;
    singleCmdHelper.packSingleCommand("Step_TC_FSGLWDXCS", packCommand, replaceParamMap);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, waitTime);
    for (auto item : cmdResultList)
    {
        if (item.m_paramdataMap.contains("TranPower"))
        {
            auto val = item.m_paramdataMap.value("TranPower").toDouble();

            pushLog(QString("发射功率均值：%1dBm").arg(val));
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
    pushLog("功率稳定性测试结束");

    // 结果入库
    AutomateTestResult automateTestResult;
    //    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;
    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "功率稳定性测试";
    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "发射功率均值(dbm)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "PST";
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

void PSTTest::stopImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    /* 停止当前测试 */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand("Step_TC_FSGLWDXCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}

void PSTTest::openLoop()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //射频模拟源闭环
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
    auto configMacro = msg.configMacroData;
    //基带设置为单载波，即调制单元载波调制方式设置为去调，伪码加调设置为去调
    //暂时用前端来设置，信号来自前端，不用基带
    DeviceID ckqdDeviceID;
    if (!configMacro.getCKQDDeviceID(m_testPlanItem.workMode, ckqdDeviceID))
    {
        pushLog("获取当前射频前端错误", true);
        return;
    }
    singleCmdHelper.packSingleCommand("Step_TC_SPQDSZQD", packCommand, ckqdDeviceID);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
    //将对应任务代号的参数宏里面的上行频率设置到前端的发射单元里任务的频率上去
    // 获取上行频率
    QMap<QString, QVariant> replaceMap;
    replaceMap["SendFreq_1"] = m_upFreq;
    singleCmdHelper.packSingleCommand("Step_TC_SPQDSZPL", packCommand, ckqdDeviceID, replaceMap);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
    //设置发射射频开关网络小环信号类型为功放小环
    singleCmdHelper.packSingleCommand("Step_TC_FSSPKGWLSZGF", packCommand);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
    //设置当前功放为去负载，并且设置功率
    DeviceID gfDeviceID;
    if (!configMacro.getSGGFDeviceID(m_testPlanItem.workMode, gfDeviceID))
    {
        pushLog("获取当前主用功放错误", true);
        return;
    }
    /*这里是主用的去负载*/
    replaceMap["SwitcherSet"] = (gfDeviceID.extenID == 1 ? 2 : 1);
    singleCmdHelper.packSingleCommand("Step_TC_GFQHKGSZQFZ", packCommand, replaceMap);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
    /*下功放的发射功率*/
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceMap[iter.key()] = iter->rawValue;
    }
    singleCmdHelper.packSingleCommand("Step_TC_GFSZFSGL", packCommand, gfDeviceID, replaceMap);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
    // S射频开关矩阵设置输出口19（频谱仪）的输入为小环监测
    singleCmdHelper.packSingleCommand("Step_TC_SPDJSKAGZSZXH", packCommand);
    if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
    {
        return;
    }
}
