#include "TSTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, STTC, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, Skuo2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, KaKuo2, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SYTHGMLSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(TSTTest, SKT, createTestItem);

QString TSTTest::typeID() { return "25_TST"; }
AutomateTestBase* TSTTest::createTestItem(AutomateTest* parent) { return new TSTTest(parent); }

TSTTest::TSTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
TSTTest::~TSTTest() {}

bool TSTTest::pretreatment()
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
bool TSTTest::linkConfiguration()
{
    /* 切换链路
     * 基带--->前端(模拟源)--->发射矩阵--->S频段测试信号(后端测试开关网络)--->频谱仪
     * 待定
     */
    if (!setConfigMacroAndParamMacro(LinkType::SZHBH))
    {
        return false;
    }

    if (!AutomateTestHelper::backEndSwitchChangeUp(*m_singleCommandHelper, this))
    {
        return false;
    }
    return true;
}
bool TSTTest::parameterSetting() { return true; }
bool TSTTest::testing(CmdResult& /*cmdResult*/)
{
    /*
     * 20210904只能做模拟源的测试
     * 在数字化闭环情况下任务输出和模拟源输出都是 模拟源的数据 此时控制测距和遥控加调是无效的  在任务模式的时候是有效的
     * 但是在任务模式的时候遥控和测距发射的接口没有对监控提供所以无法做测试
     */
    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */

    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;              /* 开始测试 */
    replaceParamMap["SpectCenter"] = 2250 * 1000; /* 中心频率 */
    replaceParamMap["ModulValue"] = m_testPlanItem.testParameterMap.value("ModulValue", AutomateTestItemParameterInfo(0.3)).rawValue;

    m_singleCommandHelper->packSingleCommand("Step_TC_TZDCS", packCommand, m_ckjd, replaceParamMap);
    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 40);
    return (execStatus == ExecStatus::Success);
}
bool TSTTest::resultOperation(const CmdResult& /*cmdResult*/)
{
    QList<double> realCmdResultList;

    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("ModulValue"))
        {
            auto val = item.m_paramdataMap.value("ModulValue").toDouble();

            pushLog(QString("调制度：%1dbm").arg(val));
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
        automateTestItemParameterInfo.displayName = "调制度(dbm)";
        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "TST";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "调制度测试";
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

void TSTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_TC_TZDCS", packCommand, m_ckjd, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
