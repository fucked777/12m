#include "PSTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, KaDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(PSTTest, SYTHGMLSK2, createTestItem);

QString PSTTest::typeID() { return "15_PST"; }
AutomateTestBase* PSTTest::createTestItem(AutomateTest* parent) { return new PSTTest(parent); }

PSTTest::PSTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
PSTTest::~PSTTest() {}

bool PSTTest::pretreatment()
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
bool PSTTest::linkConfiguration()
{
    /* 切换链路
     * S
     * (基带20210831去掉由前端发送)--->前端--->功放(小环)--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
     * ka遥测/Ka低速
     * (基带20210831去掉由前端发送)--->前端--->发射高频箱--->功放(小环)--->发射高频箱--->S频段发射矩阵--->后端测试选择开关--->后端频谱仪
     */
    return AutomateTestHelper::sendSystemLink(*m_singleCommandHelper, m_hpID, this);
}
bool PSTTest::parameterSetting() { return true; }
bool PSTTest::testing(CmdResult& /*cmdResult*/)
{
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

    auto testNumber = m_testPlanItem.testParameterMap.value("TestNumber", AutomateTestItemParameterInfo(10)).rawValue.toInt();
    auto timeInter = m_testPlanItem.testParameterMap.value("TimeInter", AutomateTestItemParameterInfo(1)).rawValue.toInt();
    if (testNumber <= 0)
    {
        pushLog("当前测试参数错误:测试次数不能为负数", true);
        return false;
    }
    if (timeInter <= 0)
    {
        pushLog("当前测试参数错误:测试时间间隔不能为负数", true);
        return false;
    }

    int waitTime = (timeInter - 1) * 60 * testNumber + 40 * testNumber;
    m_singleCommandHelper->packSingleCommand("Step_TC_FSGLWDXCS", packCommand, replaceParamMap);

    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, waitTime);
    return (execStatus == ExecStatus::Success);
}
bool PSTTest::resultOperation(const CmdResult& /*cmdResult*/)
{
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
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

    for (auto& item : realCmdResultList)
    {
        AutomateTestItemParameterInfo automateTestItemParameterInfo;
        automateTestItemParameterInfo.displayName = "发射功率均值(dbm)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "PST";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "功率稳定性测试";
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

void PSTTest::endOfTest()
{
    PackCommand packCommand;

    /* 功放关电 */
    AutomateTestHelper::hpPowerOff(*m_singleCommandHelper, this, m_hpID);

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_TC_FSGLWDXCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
