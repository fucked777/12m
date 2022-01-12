#include "RMRETTestBase.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "BusinessMacroCommon.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlAutomateTest.h"

RMRETTestBase::RMRETTestBase(AutomateTestBase* autotestbase) { m_autoTestBase = autotestbase; }

bool RMRETTestBase::setLoop()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    /* 切换链路
     * S
     * 基带--->前端--->功放--->校零变频器--->LNA--->S频段接收矩阵--->前端--->基带
     * 这里过了功放所以基带的输出就没那么重要了最终功放输出都是一样的
     */

    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;
    msg.isManualContrl = true;
    msg.configMacroData = m_autoTestBase->m_configMacroData;

    LinkLine linkLine;
    auto index = 1;
    linkLine.workMode = m_autoTestBase->m_testPlanItem.workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = m_autoTestBase->m_testPlanItem.workMode;
    targetInfo.taskCode = m_autoTestBase->m_testPlanItem.taskCode;
    targetInfo.pointFreqNo = m_autoTestBase->m_testPlanItem.dotDrequency;
    linkLine.targetMap[index] = targetInfo;
    linkLine.linkType = LinkType::PKXLBPQBH;
    msg.appendLine(linkLine);

    ControlFlowHandler controlFlowHandler;
    QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, m_autoTestBase, &AutomateTestBase::sendByteArrayToDevice);
    QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { m_autoTestBase->pushLog(msg); });
    QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { m_autoTestBase->pushLog(msg); });
    QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, [=](const QString& msg) {
        m_autoTestBase->pushLog(msg, true);
        emit m_autoTestBase->sg_end();
    });
    controlFlowHandler.handle(msg);

    auto configData = m_autoTestBase->m_configMacroData;
    if (!configData.getCKJDDeviceID(m_autoTestBase->m_testPlanItem.workMode, m_ckjd))
    {
        m_autoTestBase->pushLog("获取当前主用测控基带错误", true);
        return false;
    }

    if (!configData.getSGGFDeviceID(m_autoTestBase->m_testPlanItem.workMode, m_hpID))
    {
        m_autoTestBase->pushLog("获取当前主用高功放错误", true);
        return false;
    }

    /* 将测试信号接入测试计算机--前端频谱仪 */
    QVariantMap replace;
    replace["OutputPort19"] = (m_autoTestBase->isLeftSpin() ? 1 : 2);  //选择接入的是左旋还是右旋信号
    singleCmdHelper.packSingleCommand("Step_TC_SJSKGJZTOPPY", packCommand, replace);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    //设置发射射频开关网络小环信号类型为功放小环
    singleCmdHelper.packSingleCommand("Step_TC_FSSPKGWLSZGF", packCommand);
    if (m_autoTestBase->waitExecSuccess(packCommand, 5) != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool RMRETTestBase::setXLReduce()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /* S射频开关网络 衰减 */
    auto cmdStepKey = (m_autoTestBase->isLeftSpin() ? "Step_SBRFSN_Left_Gain" : "Step_SBRFSN_Right_Gain");
    singleCmdHelper.packSingleCommand(cmdStepKey, packCommand);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /* 校零变频器衰减 先加电再设衰减*/
    QVariantMap replace;
    for (auto iter = m_autoTestBase->m_testPlanItem.testParameterMap.begin(); iter != m_autoTestBase->m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replace[iter.key()] = iter->rawValue;
    }
    singleCmdHelper.packSingleCommand("StepSXL_STARTJD", packCommand);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    singleCmdHelper.packSingleCommand("STEPSXL_ATTENUATION", packCommand, replace);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool RMRETTestBase::saveResult(const CmdResult cmdResult)
{
    if (!cmdResult.m_paramdataMap.contains("RangMean"))
    {
        m_autoTestBase->pushLog("测试错误:当前上报结果 测距均值 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("RandDiff"))
    {
        m_autoTestBase->pushLog("测试错误:当前上报结果 测距随机差 缺失", true);
        return false;
    }
    auto rangMean = cmdResult.m_paramdataMap.value("RangMean");
    auto randDiff = cmdResult.m_paramdataMap.value("RandDiff");

    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_autoTestBase->m_testPlanItem;
    automateTestResult.testTypeID = m_autoTestBase->m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_autoTestBase->m_testPlanItem.testName;
    automateTestResult.testTypeName = m_autoTestBase->m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "测距随机误差";

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "测距均值(m)";
    automateTestItemParameterInfo.displayValue = rangMean.toString();
    automateTestItemParameterInfo.id = "RangMean";
    automateTestItemParameterInfo.rawValue = rangMean;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "测距随机差(m)";
    automateTestItemParameterInfo.displayValue = randDiff.toString();
    automateTestItemParameterInfo.id = "RandDiff";
    automateTestItemParameterInfo.rawValue = randDiff;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestResult.testResultList << automateTestResultGroup;
    automateTestResult.testTime = GlobalData::currentDateTime();

    /* 保存数据 */
    auto msg = AutomateTestHelper::toLogString(automateTestResultGroup);
    auto result = SqlAutomateTest::insertTestResultItem(automateTestResult);
    if (!result)
    {
        msg = QString("%1\n数据保存失败:%2").arg(msg, result.msg());
        m_autoTestBase->pushLog(msg, true);
        return false;
    }

    m_autoTestBase->pushLog(msg);
    m_autoTestBase->pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);

    return true;
}
