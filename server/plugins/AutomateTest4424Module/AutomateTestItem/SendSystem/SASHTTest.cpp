#include "SASHTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, SKT, createTestItem);

QString SASHTTest::typeID() { return "17_SASHT"; }
AutomateTestBase* SASHTTest::createTestItem(AutomateTest* parent) { return new SASHTTest(parent); }

SASHTTest::SASHTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
SASHTTest::~SASHTTest() {}
void SASHTTest::startImpl()
{
    pushLog("杂散及二次谐波测试开始");
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

    /* key对应客户端的配置文件 */
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }

    singleCmdHelper.packSingleCommand("Step_TC_SXZBZSCS", packCommand, replaceParamMap);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 40);
    for (auto item : cmdResultList)
    {
        if (item.m_paramdataMap.contains("SpurValue"))
        {
            auto val = item.m_paramdataMap.value("SpurValue").toDouble();

            pushLog(QString("实测杂散值：%1dBc").arg(val));
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

    singleCmdHelper.packSingleCommand("Step_TC_SXECXBCS", packCommand, replaceParamMap);
    // 测试结果
    QList<CmdResult> cmdResultList2;
    QList<double> realCmdResultList2;
    auto execStatus2 = waitExecResponceMultiData(cmdResultList, packCommand, 1, 40);
    for (auto item : cmdResultList2)
    {
        if (item.m_paramdataMap.contains("IHamVal"))
        {
            auto val = item.m_paramdataMap.value("IHamVal").toDouble();

            pushLog(QString("实测二次谐波抑制：%1dBc").arg(val));
            realCmdResultList2.append(val);
        }
        else
        {
            pushLog("配置文件解析失败，无法显示数据");
        }
    }
    if (execStatus2 != ExecStatus::Success)
    {
        return;
    }
    pushLog("二次谐波测试结束");

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
        automateTestItemParameterInfo.id = "SASHT";
        automateTestItemParameterInfo.rawValue = item;
        automateTestResultGroup.resultList << automateTestItemParameterInfo;
    }
    automateTestResult.testResultList << automateTestResultGroup;

    AutomateTestResultGroup automateTestResultGroup2;
    automateTestResultGroup2.groupName = "二次谐波测试";
    for (auto item : realCmdResultList2)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "二次谐波抑制(dBc)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "SASHT";
        automateTestItemParameterInfo.rawValue = item;
        automateTestResultGroup2.resultList << automateTestItemParameterInfo;
    }
    automateTestResult.testResultList << automateTestResultGroup2;

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

void SASHTTest::stopImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    /* 停止当前测试 */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand("Step_TC_SXZBZSCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);

    singleCmdHelper.packSingleCommand("Step_TC_SXECXBCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}

void SASHTTest::openLoop()
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
    replaceMap["TransmPower"] = 47;  //固定为50w  对应47dB
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
