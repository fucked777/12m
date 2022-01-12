#include "SASHTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
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
REGISTER_PRIVAE_CLASS_TEMPLATE(SASHTTest, KaKuo2, createTestItem);

QString SASHTTest::typeID() { return "17_SASHT"; }
AutomateTestBase* SASHTTest::createTestItem(AutomateTest* parent) { return new SASHTTest(parent); }

SASHTTest::SASHTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
SASHTTest::~SASHTTest() {}

bool SASHTTest::pretreatment()
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
bool SASHTTest::linkConfiguration()
{ /* 切换链路
   * S
   * (基带20210831去掉由前端发送)--->前端--->功放(小环)--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
   * ka遥测/Ka低速
   * (基带20210831去掉由前端发送)--->前端--->发射高频箱--->功放(小环)--->发射高频箱--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
   */
    return AutomateTestHelper::sendSystemLink(*m_singleCommandHelper, m_hpID, this);
}
bool SASHTTest::parameterSetting() { return true; }
bool SASHTTest::testing(CmdResult& /*cmdResult*/)
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

    // 待定这里应该测试两个命令
    // Step_TC_SXZBZSCS
    m_singleCommandHelper->packSingleCommand("Step_TC_SXECXBCS", packCommand, replaceParamMap);

    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 40);
    return (execStatus == ExecStatus::Success);
}
bool SASHTTest::resultOperation(const CmdResult& /*cmdResult*/)
{
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("IHamVal"))
        {
            auto val = item.m_paramdataMap.value("IHamVal").toDouble();

            pushLog(QString("实测二次谐波抑制：%1dBc").arg(val));
            realCmdResultList.append(val);
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

    for (auto item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "二次谐波抑制(dBc)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "SASHT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "杂散及二次谐波";
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

void SASHTTest::endOfTest()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /* 功放关电 */
    AutomateTestHelper::hpPowerOff(singleCmdHelper, this, m_hpID);

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand("Step_TC_SXECXBCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
