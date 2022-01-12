#include "RPNTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(RPNTTest, XDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RPNTTest, KaGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RPNTTest, KaDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RPNTTest, KaKuo2, createTestItem);

QString RPNTTest::typeID() { return "19_RPNT"; }
AutomateTestBase* RPNTTest::createTestItem(AutomateTest* parent) { return new RPNTTest(parent); }

RPNTTest::RPNTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
RPNTTest::~RPNTTest() {}

bool RPNTTest::linkConfiguration()
{
    /* 切换链路
     * X/ka遥测/ka低速/Ka高速
     * 高频箱本振--->前端测试信号选择开关--->前端频谱仪
     */

    if (!setConfigMacroAndParamMacro(LinkType::SPMNYYXBH))
    {
        return false;
    }

    /* 切开关下行本振接入前端测试选择开关 */
    if (!AutomateTestHelper::frontLocalVibrationEndSwitchChange(*m_singleCommandHelper, this))
    {
        return false;
    }
    return true;
}
bool RPNTTest::parameterSetting()
{
    /* 本振信号选择
     * 这里是信号源输出的
     * 无论左旋还是右旋都是全部接入的
     */
    if (!AutomateTestHelper::localVibrationTestSignChoose(*m_singleCommandHelper, this))
    {
        return false;
    }
    return true;
}
bool RPNTTest::testing(CmdResult& cmdResult)
{
    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;     /* 开始测试 */
    replaceParamMap["SpectSelect"] = 2;  /* 频谱仪1后端2前端 */
    replaceParamMap["OutputSwitch"] = 1; /* 信号源关 0on 1off */
    if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
    {
        /* 理论上是二级变频
         * 但是这里不知道是一级还是二级的
         */
        pushLog("暂时不知道KaKuo2本振的计算方法", true);
        return false;
    }
    if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
    {
        /* 理论上是二级变频
         * 但是这里不知道是一级还是二级的
         */
        pushLog("暂时不知道KaDS本振的计算方法", true);
        return false;
    }
    if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    {
        /* 高速给的信号可以是1级本振 也可以是2级
         * 一级是固定的5.4G
         * 二级是射频-5.4g-1.2g
         */
        pushLog("暂时不知道KaGS本振的计算方法", true);
        return false;
    }
    else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::XBand)
    {
        /* X本振的计算方式是
         * fLO=f0-1200MHz
         *
         * X也是二级变频
         * 一级的频率位置
         * 这里也要判断测试的是一本振还是二本振
         */
        replaceParamMap["SpectCenter"] = m_downFreq - 2250; /* 中心频率 */
        pushLog("暂时不知道XDS本振的计算方法", true);
        return false;
    }

    PackCommand packCommand;
    m_singleCommandHelper->packSingleCommand("Step_TC_XXXWZSCS", packCommand, replaceParamMap);
    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 140);
    return (execStatus == ExecStatus::Success);
}
bool RPNTTest::resultOperation(const CmdResult& cmdResult)
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
        automateTestItemParameterInfo.id = "RPNT";
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

void RPNTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_TC_XXXWZSCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
