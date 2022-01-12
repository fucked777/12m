#include "FRTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, KaKuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, XDS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, KaGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, KaDS, createTestItem);

// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHSMJ, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHWX, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHGML, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHSMJK2BGZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHSMJK2GZB, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHWXSK2, createTestItem);
// REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHGMLSK2, createTestItem);

QString FRTTest::typeID() { return "21_FRT"; }
AutomateTestBase* FRTTest::createTestItem(AutomateTest* parent) { return new FRTTest(parent); }

FRTTest::FRTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
FRTTest::~FRTTest() {}
bool FRTTest::linkConfiguration()
{
    /* 切换链路
     * X/Ka低速/Ka高速/Ka遥测
     * 信号源--->前端测试开关网络--->LNA--->高频箱(本振)--->前端信号选择开关--->前端频谱仪
     * S
     * 信号源--->前端测试开关网络--->LNA--->前端信号选择开关--->前端频谱仪
     */
    if (!setConfigMacroAndParamMacro(LinkType::SPMNYYXBH))
    {
        return false;
    }

    /* 信号源接入测试高频箱 */
    if (!AutomateTestHelper::frontSignalSourceToLNA(*m_singleCommandHelper, this))
    {
        return false;
    }

    /* 切换开关下行本振接入前端测试选择开关 */
    if (!AutomateTestHelper::frontLocalVibrationEndSwitchChange(*m_singleCommandHelper, this))
    {
        return false;
    }
    return true;
}
bool FRTTest::parameterSetting()
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
bool FRTTest::testing(CmdResult& cmdResult)
{
    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    PackCommand packCommand;
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;            /* 开始测试 */
    replaceParamMap["SignalFreq"] = m_downFreq; /* 信号频率 */
    replaceParamMap["SpectSelect"] = 2;         /* 频谱仪1后端2前端 */
    replaceParamMap["OutputSwitch"] = 0;        /* 信号源关 0on 1off */
    replaceParamMap["SignalSelect"] = 2;        /* 1后端2前端 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SBand)
    {
        replaceParamMap["SpectCenter"] = m_downFreq; /* 中心频率 */
        /* 获取ini里面的扫频的范围 */
        auto signalStartRange = m_singleCommandHelper->getSingleCmdValue("Step_TC_XXLLPLXYCS", "SignalStartCenter", 760).toInt();
        auto SignalEndRange = m_singleCommandHelper->getSingleCmdValue("Step_TC_XXLLPLXYCS", "SignalEndCenter", 760).toInt();
        replaceParamMap["SignalStartCenter"] = m_downFreq - signalStartRange; /* 信号扫描始频率 */
        replaceParamMap["SignalEndCenter"] = m_downFreq + SignalEndRange;     /* 信号扫描终频率 */
    }
    else if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
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

    m_singleCommandHelper->packSingleCommand("Step_TC_XXLLPLXYCS", packCommand, replaceParamMap);
    // 测试结果
    m_cmdResultList.clear();
    auto execStatus = waitExecResponceMultiData(m_cmdResultList, packCommand, 1, 40);
    return (execStatus == ExecStatus::Success);
}
bool FRTTest::resultOperation(const CmdResult& cmdResult)
{
    QList<double> realCmdResultList;
    for (auto& item : m_cmdResultList)
    {
        if (item.m_paramdataMap.contains("1Bandwidth") && item.m_paramdataMap.contains("3Bandwidth") &&
            item.m_paramdataMap.contains("GainFlatJustAdd") && item.m_paramdataMap.contains("CorreFreqJust") &&
            item.m_paramdataMap.contains("GainFlatNeg") && item.m_paramdataMap.contains("CorreFreqneg") &&
            item.m_paramdataMap.contains("GainFlatJust"))
        {
            auto Bandwidth1 = item.m_paramdataMap.value("1Bandwidth").toDouble();
            auto Bandwidth3 = item.m_paramdataMap.value("3Bandwidth").toDouble();
            auto GainFlatJustAdd = item.m_paramdataMap.value("GainFlatJustAdd").toDouble();
            auto CorreFreqJust = item.m_paramdataMap.value("CorreFreqJust").toDouble();
            auto GainFlatNeg = item.m_paramdataMap.value("GainFlatNeg").toDouble();
            auto CorreFreqneg = item.m_paramdataMap.value("CorreFreqneg").toDouble();
            auto GainFlatJust = item.m_paramdataMap.value("GainFlatJust").toDouble();

            pushLog(QString("1dB带宽：%1 Hz").arg(Bandwidth1));
            pushLog(QString("3dB带宽：%1 Hz").arg(Bandwidth3));
            pushLog(QString("增益平坦度(正)：%1dB").arg(GainFlatJustAdd));
            pushLog(QString("对应频率：%1 Hz").arg(CorreFreqJust));
            pushLog(QString("增益平坦度(负)：%1dB").arg(GainFlatNeg));
            pushLog(QString("对应频率：%1Hz").arg(CorreFreqneg));
            pushLog(QString("增益平坦度：%1dB").arg(GainFlatJust));
            realCmdResultList.append(Bandwidth1);
            realCmdResultList.append(Bandwidth3);
            realCmdResultList.append(GainFlatJustAdd);
            realCmdResultList.append(CorreFreqJust);
            realCmdResultList.append(GainFlatNeg);
            realCmdResultList.append(CorreFreqneg);
            realCmdResultList.append(GainFlatJust);
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
            automateTestItemParameterInfo.displayName = "1dB带宽(Hz)";
        if (index == 2)
            automateTestItemParameterInfo.displayName = "3dB带宽(Hz)";
        if (index == 3)
            automateTestItemParameterInfo.displayName = "增益平坦度(正)(dB)";
        if (index == 4)
            automateTestItemParameterInfo.displayName = "对应频率(Hz)";
        if (index == 5)
            automateTestItemParameterInfo.displayName = "增益平坦度(负)(dB)";
        if (index == 6)
            automateTestItemParameterInfo.displayName = "对应频率(Hz)";
        if (index == 7)
            automateTestItemParameterInfo.displayName = "增益平坦度(dB)";
        index++;

        automateTestItemParameterInfo.displayValue = QString::number(item, 'f', 3);
        automateTestItemParameterInfo.id = "FRT";
        automateTestItemParameterInfo.rawValue = item;
        AutomateTestResultGroup automateTestResultGroup;
        automateTestResultGroup.groupName = "频率响应测试";
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

void FRTTest::endOfTest()
{
    PackCommand packCommand;

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    m_singleCommandHelper->packSingleCommand("Step_TC_XXLLPLXYCS", packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
