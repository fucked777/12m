#include "LBTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, KaKuo2, createTestItem);

// 扩跳无此测试
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SKT, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(LBTTest, SYTHGMLSK2, createTestItem);

QString LBTTest::typeID() { return "27_LBT"; }
AutomateTestBase* LBTTest::createTestItem(AutomateTest* parent) { return new LBTTest(parent); }

LBTTest::LBTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
LBTTest::~LBTTest() {}

bool LBTTest::pretreatment()
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

    return true;
}

bool LBTTest::linkConfiguration()
{
    /* 切换链路
     * 中频闭环
     * 基带--->前端(模拟源)--->发射矩阵--->接收矩阵--->前端
     */
    return setConfigMacroAndParamMacro(LinkType::SZHBH);
}
bool LBTTest::parameterSetting() { return true; }
bool LBTTest::testing(CmdResult& /*cmdResult*/)
{
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    replaceParamMap["RotatDirect"] = isLeftSpin() ? 1 : 2;

    if (this->m_testPlanItem.workMode == SystemWorkMode::STTC)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_LBT_1", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::Skuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_LBT_4", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_LBT_5", packCommand, m_ckjd, replaceParamMap);
    }

    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 80);
    return (execStatus == ExecStatus::Success);
}
bool LBTTest::resultOperation(const CmdResult& /*cmdResult*/)
{
    QList<double> realCmdResultList;

    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("Bandwidth5") && item.m_paramdataMap.contains("Bandwidth10") &&
            item.m_paramdataMap.contains("Bandwidth100") && item.m_paramdataMap.contains("Bandwidth500") &&
            item.m_paramdataMap.contains("Bandwidth1000") && item.m_paramdataMap.contains("Bandwidth2000"))
        {
            auto testStates = item.m_paramdataMap.value("TestStatus").toInt();
            auto testState = item.m_paramdataMap.value("TestState").toInt();
            //如果是测试完成就推送结果
            if (testState == 2 || testStates == 2 || testState == 1 || testStates == 1)
            {
                auto Bandwidth5 = item.m_paramdataMap.value("Bandwidth5").toDouble();
                auto Bandwidth10 = item.m_paramdataMap.value("Bandwidth10").toDouble();
                auto Bandwidth100 = item.m_paramdataMap.value("Bandwidth100").toDouble();
                auto Bandwidth500 = item.m_paramdataMap.value("Bandwidth500").toDouble();
                auto Bandwidth1000 = item.m_paramdataMap.value("Bandwidth1000").toDouble();
                auto Bandwidth2000 = item.m_paramdataMap.value("Bandwidth2000").toDouble();

                pushLog(QString("带宽5Hz测试值%1Hz").arg(Bandwidth5));
                pushLog(QString("带宽10Hz测试值%1Hz").arg(Bandwidth10));
                pushLog(QString("带宽100Hz测试值%1Hz").arg(Bandwidth100));
                pushLog(QString("带宽500Hz测试值%1Hz").arg(Bandwidth500));
                pushLog(QString("带宽1000Hz测试值%1Hz").arg(Bandwidth1000));
                pushLog(QString("带宽2000Hz测试值%1Hz").arg(Bandwidth2000));
                realCmdResultList.append(Bandwidth5);
                realCmdResultList.append(Bandwidth10);
                realCmdResultList.append(Bandwidth100);
                realCmdResultList.append(Bandwidth500);
                realCmdResultList.append(Bandwidth1000);
                realCmdResultList.append(Bandwidth2000);
            }
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
            automateTestItemParameterInfo.displayName = QString("带宽5Hz测试值");
        if (index == 2)
            automateTestItemParameterInfo.displayName = QString("带宽10Hz测试值");
        if (index == 3)
            automateTestItemParameterInfo.displayName = QString("带宽100Hz测试值");
        if (index == 4)
            automateTestItemParameterInfo.displayName = QString("带宽500Hz测试值");
        if (index == 5)
            automateTestItemParameterInfo.displayName = QString("带宽1000Hz测试值");
        if (index == 6)
            automateTestItemParameterInfo.displayName = QString("带宽2000Hz测试值");
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "TFTT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = QString("等效环路带宽 ").arg(qPow(10, index % 7) / 10);
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

void LBTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    if (this->m_testPlanItem.workMode == SystemWorkMode::STTC)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_LBT_1", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::Skuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_LBT_4", packCommand, m_ckjd, replaceParamMap);
    }
    else if (this->m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        m_singleCommandHelper->packSingleCommand("Step_AUTOTEST_LBT_5", packCommand, m_ckjd, replaceParamMap);
    }
    notWaitExecSuccessBeQuiet(packCommand);
}
