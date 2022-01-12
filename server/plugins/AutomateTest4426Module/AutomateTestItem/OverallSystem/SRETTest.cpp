#include "SRETTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "ParamMacroMessageSerialize.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, KaKuo2, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SYTHGMLSK2, createTestItem);

// 扩跳无此测试项
// REGISTER_PRIVAE_CLASS_TEMPLATE(SRETTest, SKT, createTestItem);

QString SRETTest::typeID() { return "5_SRET"; }
AutomateTestBase* SRETTest::createTestItem(AutomateTest* parent) { return new SRETTest(parent); }

SRETTest::SRETTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
SRETTest::~SRETTest() {}
bool SRETTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
        return false;
    }
    if (!m_configMacroData.getCKJDDeviceID(m_testPlanItem.workMode, m_ckjd, 1))
    {
        pushLog("获取当前主用测控基带错误", true);
        return false;
    }

    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        if (!m_configMacroData.getSGGFDeviceID(m_testPlanItem.workMode, m_hpID))
        {
            pushLog("获取当前主用功放错误", true);
            return false;
        }
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        if (!m_configMacroData.getKaGGFDeviceID(m_testPlanItem.workMode, m_hpID))
        {
            pushLog("获取当前主用功放错误", true);
            return false;
        }
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
    {
        if (!m_configMacroData.getKaDtGGFDeviceID(m_testPlanItem.workMode, m_hpID))
        {
            pushLog("获取当前主用功放错误", true);
            return false;
        }
    }

    return true;
}

bool SRETTest::linkConfiguration()
{
    /*
     * 20210827 wp?
     * 已经询问过闫总  颜总
     * 现在将此测试项设置和24的流程设置为一样
     */
    /* 切换链路
     * S
     * 基带--->前端--->功放--->校零变频器--->LNA--->S频段接收矩阵--->前端--->基带
     * ka遥测
     * 基带--->前端--->功放--->校零变频器--->LNA--->高频箱--->S频段接收矩阵--->前端--->基带
     * 这里过了功放所以基带的输出就没那么重要了最终功放输出都是一样的
     */
    return setConfigMacroAndParamMacro(LinkType::PKXLBPQBH);
}
bool SRETTest::parameterSetting()
{
    PackCommand packCommand;
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    /* 参数设置 */

    /* 基带 */
    if (m_testPlanItem.workMode == SystemWorkMode::STTC)
    {
        /* 基带测距加调
         * 主音调制度0.3rad
         * 次音调制度0.3rad
         */
        m_singleCommandHelper->packSingleCommand("StepCKJD_STTC_IFCM_AUTOTEST_CJ", packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        /* 最低侧音8Hz */
        auto cmdStepKey = (isLeftSpin() ? "StepCKJD_STTC_LTRU_AUTOTEST_MinSide" :  //
                               "StepCKJD_STTC_RTRU_AUTOTEST_MinSide");
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::Skuo2 || m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        /* 设置测控基带目标1
         * 接收单元遥测支路伪码码率为10230kc
         */
        auto cmdStepKey = QString("StepCKJD_%1_TIRU_AUTOTEST_SpreadCode").arg(cmdStepFormatArg);
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        /* 设置测控基带目标1
         * Gold码参数如下：多项式A：197H，初相A：35AH，多项式B：81H，初相B：248H
         */
        cmdStepKey = QString("StepCKJD_%1_SSCU_AUTOTEST_PNCodeMulti").arg(cmdStepFormatArg);
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        /* 设置测控基带目标1
         * 遥测码率2kbps，码型NRZ-L，
         */
        cmdStepKey = QString("StepCKJD_%1_TU_AUTOTEST_Code").arg(cmdStepFormatArg);
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        /* 设置测控基带中频调制单元目标1遥测支路加调，其它目标去调，控制测控基带输出单载波
         * 20210830 wp?? 没有遥测支路 这里先都加调
         *
         * 先全部去调 然后单独的将通道1加调
         */
        cmdStepKey = QString("StepCKJD_%1_JTKZ_AUTOTEST").arg(cmdStepFormatArg);
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        cmdStepKey = QString("StepCKJD_%1_IFCM_AUTOTEST_Add").arg(cmdStepFormatArg);
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand, m_ckjd);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }

    /* 链路衰减 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        /* S射频开关网络 增益 */
        auto cmdStepKey = (isLeftSpin() ? "Step_SBRFSN_Left_Gain" : "Step_SBRFSN_Right_Gain");
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        /* 校零变频器衰减 */
        m_singleCommandHelper->packSingleCommand("STEPSXL_ATTENUATION", packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
    {
        /* 校零变频器衰减 */
        m_singleCommandHelper->packSingleCommand("StepKaXL_ATTENUATION", packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        /* 高频箱衰减 */
        auto cmdStepKey = (isLeftSpin() ? "Step_KaMCATHFB_KAS1_Attenuation" : "Step_KaMCATHFB_KAS2_Attenuation");
        m_singleCommandHelper->packSingleCommand(cmdStepKey, packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    return true;
}
bool SRETTest::testing(CmdResult& cmdResult)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;    /* 开始测试 */
    replaceParamMap["RotatDirect"] = 1; /* 旋向 */
    /* 统计点数 */
    auto statPoint = m_testPlanItem.testParameterMap.value("StatPoint").rawValue.toInt();
    if (statPoint <= 0)
    {
        pushLog("测试错误:当前统计及点数是非法值", true);
        return false;
    }
    replaceParamMap["StatPoint"] = statPoint;

    if (m_testPlanItem.workMode != SystemWorkMode::STTC)
    {
        replaceParamMap["ChannelNumb"] = 1; /* 选通道1 */
    }

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    m_singleCommandHelper->packSingleCommand(QString("StepCKJD_%1_AUTOTESTSRET").arg(cmdStepFormatArg), packCommand, m_ckjd, replaceParamMap);
    auto execStatus =
        waitExecResponceCondition(cmdResult, packCommand, 3, 1200, statPoint, [](CmdResult& cmdResult, QString& errMsg, const QVariant& arg) {
            if (!cmdResult.m_paramdataMap.contains("StatCount"))
            {
                errMsg = "上报结果无已统计点数的数据";
                return ConditionResult::Error;
            }
            auto statCount = cmdResult.m_paramdataMap.value("StatCount").toInt();
            return (statCount >= arg.toInt() ? ConditionResult::OK : ConditionResult::Wait);
        });
    return (execStatus == ExecStatus::Success);
}
bool SRETTest::resultOperation(const CmdResult& cmdResult)
{
    if (!cmdResult.m_paramdataMap.contains("VelMeanVal"))
    {
        pushLog("测试错误:当前上报结果 测速均值 缺失", true);
        return false;
    }
    if (!cmdResult.m_paramdataMap.contains("RandDiff"))
    {
        pushLog("测试错误:当前上报结果 测速随机差 缺失", true);
        return false;
    }

    auto velMeanVal = cmdResult.m_paramdataMap.value("VelMeanVal");
    auto randDiff = cmdResult.m_paramdataMap.value("RandDiff");

    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "测速随机误差";

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "测速均值(m/s)";
    automateTestItemParameterInfo.displayValue = velMeanVal.toString();
    automateTestItemParameterInfo.id = "VelMeanVal";
    automateTestItemParameterInfo.rawValue = velMeanVal;
    automateTestResultGroup.resultList << automateTestItemParameterInfo;

    automateTestItemParameterInfo.displayName = "测速随机差(m/s)";
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
        pushLog(msg, true);
        return false;
    }

    pushLog(msg);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);

    return true;
}

void SRETTest::endOfTest()
{
    PackCommand packCommand;

    /* 校零变频器关电 */
    AutomateTestHelper::xlbpqPowerOff(*m_singleCommandHelper, this);
    /* 功放关电 */
    AutomateTestHelper::hpPowerOff(*m_singleCommandHelper, this, m_hpID);

    /* 停止当前测试 */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand(QString("StepCKJD_%1_AUTOTESTSRET").arg(cmdStepFormatArg), packCommand, m_ckjd, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
