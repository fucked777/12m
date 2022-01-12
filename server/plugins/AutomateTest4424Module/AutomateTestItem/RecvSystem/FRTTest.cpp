#include "FRTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestSerialize.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, XGS, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(FRTTest, KaGS, createTestItem);

QString FRTTest::typeID() { return "21_FRT"; }
AutomateTestBase* FRTTest::createTestItem(AutomateTest* parent) { return new FRTTest(parent); }

FRTTest::FRTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
FRTTest::~FRTTest() {}
void FRTTest::startImpl()
{
    pushLog("频率响应测试开始");
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
    replaceParamMap["TestCtrl"] = 1;             /* 开始测试 */
    replaceParamMap["SpectCenter"] = m_downFreq; /* 中心频率 */
    if (m_testPlanItem.workMode == XGS)
    {
        replaceParamMap["SpectSpan"] = 1320000; /* X频段频谱仪SPAN固定为1320M */
        replaceParamMap["SpectSelect"] = 1;     /* 频谱仪  1:后端  2:前端 */
    }
    else if (m_testPlanItem.workMode == KaGS)
    {
        replaceParamMap["SpectSpan"] = 1520000; /* Ka频段频谱仪SPAN固定为1520M */
        replaceParamMap["SpectSelect"] = 1;     /* 频谱仪  1:后端  2:前端 */
    }
    else
    {
        replaceParamMap["SpectSpan"] = 40000; /* S频段频谱仪SPAN固定为40M */
        replaceParamMap["SpectSelect"] = 2;   /* 频谱仪  1:后端  2:前端 */
    }

    /* key对应客户端的配置文件 */
    for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
    {
        replaceParamMap[iter.key()] = iter->rawValue;
    }

    singleCmdHelper.packSingleCommand("Step_TC_XXLLPLXYCS", packCommand, replaceParamMap);
    // 测试结果
    QList<CmdResult> cmdResultList;
    QList<double> realCmdResultList;
    auto execStatus = waitExecResponceMultiData(cmdResultList, packCommand, 1, 40);

    for (auto item : cmdResultList)
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
        }
    }

    if (execStatus != ExecStatus::Success)
    {
        return;
    }
    pushLog("频率响应测试结束");

    // 结果入库
    AutomateTestResult automateTestResult;
    //    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "频率响应测试";
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

        automateTestResultGroup.resultList << automateTestItemParameterInfo;
    }
    automateTestResult.testResultList << automateTestResultGroup;
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

void FRTTest::stopImpl() {}

void FRTTest::openLoop()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // TODO 配置射频模拟源闭环
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

    if (m_testPlanItem.workMode == XGS || m_testPlanItem.workMode == KaGS)
    {
        //信号源接入后端测试开关网络
        singleCmdHelper.packSingleCommand("Step_TC_XHYTOHZCSKGWL", packCommand);
        if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
        {
            return;
        }

        // 1.2G中频开关矩阵输出口7选择下变频器
        QVariantMap replaceParamMap;
        for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
        {
            replaceParamMap[iter.key()] = iter->rawValue;
        }
        singleCmdHelper.packSingleCommand("Step_TC_ZPKGJZXZXBPQX", packCommand, replaceParamMap);
        if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
        {
            return;
        }
        //后端测试开关网络接入频谱仪  1.2GHz监测信号
        singleCmdHelper.packSingleCommand("Step_TC_HZCSKGWLTOPPY2", packCommand);
        if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
        {
            return;
        }
    }
    //除了X高速和Ka高速就只有S频段的了
    else
    {
        // S频段接收开关矩阵  频谱仪接入左右旋信号
        QVariantMap replaceParamMap;
        for (auto iter = m_testPlanItem.testParameterMap.begin(); iter != m_testPlanItem.testParameterMap.end(); ++iter)
        {
            replaceParamMap[iter.key()] = iter->rawValue;
        }
        singleCmdHelper.packSingleCommand("Step_TC_SJSKGJZTOPPY", packCommand, replaceParamMap);
        if (waitExecSuccess(packCommand, 5) != ExecStatus::Success)
        {
            return;
        }
    }
}
