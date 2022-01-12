#include "LPNTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, KaDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LPNTTest, SYTHGMLSK2, createTestItem);

QString LPNTTest::typeID() { return "16_LPNT"; }
AutomateTestBase* LPNTTest::createTestItem(AutomateTest* parent) { return new LPNTTest(parent); }

LPNTTest::LPNTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
LPNTTest::~LPNTTest() {}

bool LPNTTest::pretreatment()
{
    if (!AutomateTestBase::pretreatment())
    {
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
bool LPNTTest::linkConfiguration()
{
    /* 切换链路
     * S
     * (基带20210831去掉由前端发送)--->前端--->功放(小环)--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
     * ka遥测/Ka低速
     * (基带20210831去掉由前端发送)--->前端--->发射高频箱--->功放(小环)--->发射高频箱--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
     */
    return AutomateTestHelper::sendSystemLink(*m_singleCommandHelper, m_hpID, this);
}
bool LPNTTest::parameterSetting() { return true; }
bool LPNTTest::testing(CmdResult& /*cmdResult*/)
{
    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        replaceParamMap["SpectCenter"] = m_upFreq; /* 中心频率 */
    }
    else
    {
        replaceParamMap["SpectCenter"] = 2250 * 1000;
    }
    replaceParamMap["SpectSelect"] = 1;  /* 频谱仪1后端2前端 */
    replaceParamMap["OutputSwitch"] = 1; /* 信号源关 0on 1off */

    m_singleCommandHelper->packSingleCommand("Step_TC_SXZBXWZSCS", packCommand, replaceParamMap);

    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 150);
    return (execStatus == ExecStatus::Success);
}
bool LPNTTest::resultOperation(const CmdResult& cmdResult)
{
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
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
            return false;
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
        automateTestItemParameterInfo.id = "LPNT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "相位噪声测试";
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
        return false;
    }

    pushLog("数据保存成功");

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
    return true;
}

void LPNTTest::endOfTest()
{
    PackCommand packCommand;

    /* 功放关电 */
    AutomateTestHelper::hpPowerOff(*m_singleCommandHelper, this, m_hpID);

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_TC_SXZBXWZSCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
