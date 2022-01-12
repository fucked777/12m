#include "GTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "SingleCommandHelper.h"
#include "SqlAutomateTest.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SKT, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(GTTest, SYTHGMLSK2, createTestItem);

QString GTTest::typeID() { return "1_GT"; }
AutomateTestBase* GTTest::createTestItem(AutomateTest* parent) { return new GTTest(parent); }

GTTest::GTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
GTTest::~GTTest() {}
void GTTest::stopImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    /* 校零变频器关电 */  //这里需要修改，24的校零开关电和26不一样(等待新协议)
    AutomateTestHelper::xlbpqPowerOff(singleCmdHelper, this);

    /* 停止当前测试 */
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    auto stepKey = "Step_TC_SBF";    /*(m_testPlanItem.workMode == SystemWorkMode::KaGS ? "Step_TC_SBF" : "Step_TC_SBF_1_2G")*/
    singleCmdHelper.packSingleCommand(stepKey, packCommand, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
void GTTest::startImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    pushLog("GT值测试开始");
    pushStatus(AutomateTestStatus::LinkConfiguration);
    /* 切换链路
     * G/T值测试  因为S校零只有在S频段射频开关网络有，所有G/T值测试只能测试S频段的
     * S
     * 校零变频器--->LNA--->开关网络--->S频段接收矩阵--->信号选择开关--->后端频谱仪
     *
     * 这里只是复用链路切换的代码
     * G/T是连频率都要更换的
     */
    /* 这个频率要变成装订的频率 */
    m_downFreq = m_testPlanItem.automateTestParameterBind.testBindParameterMap["F0"].rawValue.toDouble();
    {
        ManualMessage msg;
        msg.manualType = ManualType::ConfigMacroAndParamMacro;
        msg.isManualContrl = true;
        msg.configMacroData = m_configMacroData;

        LinkLine linkLine;
        auto index = 1;
        linkLine.workMode = m_testPlanItem.workMode;
        linkLine.masterTargetNo = index;
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = m_testPlanItem.workMode;
        targetInfo.taskCode = m_testPlanItem.taskCode;
        targetInfo.pointFreqNo = m_testPlanItem.dotDrequency;
        linkLine.targetMap[index] = targetInfo;
        linkLine.linkType = LinkType::PKXLBPQBH;
        msg.appendLine(linkLine);

        ControlFlowHandler controlFlowHandler;
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &GTTest::sendByteArrayToDevice);
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, [=](const QString& msg) {
            this->pushLog(msg, true);
            emit this->sg_end();
        });
        controlFlowHandler.handle(msg);
    }

    /* 将测试信号接入测试计算机--前端频谱仪 */
    {
        QVariantMap replace;
        replace["OutputPort19"] = (isLeftSpin() ? 1 : 2);  //选择接入的是左旋还是右旋信号
        singleCmdHelper.packSingleCommand("Step_TC_SJSKGJZTOPPY", packCommand, replace);
        auto execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }
    }

    pushStatus(AutomateTestStatus::ParameterSetting);

    /* 参数设置
     * 这里要设置的有
     * 1.改变校零变频器输出频率为参数装订的F0  改变衰减为参数装订的A  切换为信标模式
     * 2.改变高频箱的工作频率为F0 衰减
     */
    {
        /* 改变校零变频器输出频率为参数装订的F0  改变衰减为参数装订的A  切换为信标模式 开电 */
        auto& bindMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
        QVariantMap replaceParamMap;
        replaceParamMap["BeconFreq"] = m_downFreq; /* 最上方已经改成了装订的频率 */
        replaceParamMap["BeconReduce"] = bindMap["XLAttenuation"].rawValue;

        // S校零加电
        singleCmdHelper.packSingleCommand("StepSXL_STARTJD", packCommand);
        auto execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }

        if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
        {
            //下发校零频率和衰减
            singleCmdHelper.packSingleCommand("StepSXB_START", packCommand, replaceParamMap);
            execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        //        else if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::KaBand)
        //        {
        //            singleCmdHelper.packSingleCommand("StepKaXB_START", packCommand, replaceParamMap);
        //        }

        //        replaceParamMap.clear();
        //        replaceParamMap["SBandDownFreq"] = bindMap["F0"].rawValue;
        //        replaceParamMap["RadioFrequDecrement_A"] = bindMap["GPXAttenuation"].rawValue;

        /* 改变高频箱的工作频率为F0 S不需要变频所以不用改(现暂时没有Ka频段的校零，所有不对Ka和X频段进行操作) */
        //        if (m_testPlanItem.workMode == SystemWorkMode::KaKuo2)
        //        {
        //            singleCmdHelper.packSingleCommand((isLeftSpin() ? "Step_kackgzgpx_kaFreqAndgain_5" : "Step_kackgzgpx_kaFreqAndgain_6"),
        //            packCommand,
        //                                              replaceParamMap);
        //        }
        //        else if (m_testPlanItem.workMode == SystemWorkMode::KaDS)
        //        {
        //            singleCmdHelper.packSingleCommand((isLeftSpin() ? "Step_KaDSTrack_gain_5" : "Step_KaDSTrack_gain_6"), packCommand,
        //            replaceParamMap);
        //        }
        //        else if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
        //        {
        //            singleCmdHelper.packSingleCommand((isLeftSpin() ? "Step_KaGSSCGPX_freq_gain_3" : "Step_KaGSSCGPX_freq_gain_4"), packCommand,
        //                                              replaceParamMap);
        //        }

        execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }
    }
    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1; /* 开始测试 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand)
    {
        replaceParamMap["SpectCenter"] = m_downFreq * 1000; /* 中心频率 */
    }
    //    else if (m_testPlanItem.workMode == SystemWorkMode::KaGS)
    //    {
    //        replaceParamMap["SpectCenter"] = 1200 * 1000;
    //    }
    //    else
    //    {
    //        replaceParamMap["SpectCenter"] = 2250 * 1000;
    //    }

    replaceParamMap["SpectSelect"] = 2;  /* 选择前端频谱仪 */
    replaceParamMap["OutputSwitch"] = 1; /* 信号源关 0on 1off */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    auto stepKey = "Step_TC_SBF"; /*(m_testPlanItem.workMode == SystemWorkMode::KaGS ? "Step_TC_SBF" : "Step_TC_SBF_1_2G")*/
    singleCmdHelper.packSingleCommand(stepKey, packCommand, replaceParamMap);
    CmdResult cmdResult;
    auto execStatus = waitExecResponceData(cmdResult, packCommand, 3, 40);
    if (execStatus != ExecStatus::Success)
    {
        return;
    }

    auto curTestResultfind = cmdResult.m_paramdataMap.find("S/φRealValu");
    if (curTestResultfind == cmdResult.m_paramdataMap.end())
    {
        pushLog("测试错误:未能获取当前的S/φ值", true);
        return;
    }
    auto testResult = curTestResultfind->toDouble();
    /* 拿到S/φ的结果了 */
    /* G/T = G/T0 +（S/Φx - S/Φ0）
     * 就是G/t值等于 人工标校的G/T0加上 当前测试的S/Φx与人工标校S/Φ之差
     */
    auto& bindValueMap = m_testPlanItem.automateTestParameterBind.testBindParameterMap;
    auto bindFind = bindValueMap.find("SBF");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订S/Φ0参数错误", true);
        return;
    }
    auto sbf0 = bindFind->rawValue.toDouble();

    bindFind = bindValueMap.find("GT0");
    if (bindFind == bindValueMap.end())
    {
        pushLog("参数缺失:查找参数装订GT0参数错误", true);
        return;
    }
    auto gt0 = bindFind->rawValue.toDouble();

    auto desGT = gt0 + (testResult - sbf0);
    // S/φRealValu  S/φ实测值
    AutomateTestResult automateTestResult;
    /* 这么创建ID是方便搜索 */
    automateTestResult.id = Utility::createTimeUuid();
    automateTestResult.testParam = m_testPlanItem;
    automateTestResult.testTypeID = m_testPlanItem.testTypeID;
    automateTestResult.testPlanName = m_testPlanItem.testName;
    automateTestResult.testTypeName = m_testPlanItem.testTypeName;

    AutomateTestItemParameterInfo automateTestItemParameterInfo;
    automateTestItemParameterInfo.displayName = "G/T值";
    automateTestItemParameterInfo.displayValue = QString::number(desGT, 'f', 3);
    automateTestItemParameterInfo.id = "G/T";
    automateTestItemParameterInfo.rawValue = desGT;
    AutomateTestResultGroup automateTestResultGroup;
    automateTestResultGroup.groupName = "G/T值";
    automateTestResultGroup.resultList << automateTestItemParameterInfo;
    automateTestResult.testResultList << automateTestResultGroup;

    automateTestResult.testTime = GlobalData::currentDateTime();

    /* 保存数据 */
    auto msg = AutomateTestHelper::toLogString(automateTestResultGroup);
    msg += QString("\nS/φ:%1").arg(testResult, 0, 'f', 3);
    auto result = SqlAutomateTest::insertTestResultItem(automateTestResult);
    if (!result)
    {
        msg = QString("%1\n数据保存失败:%2").arg(msg, result.msg());
        pushLog(msg, true);
        return;
    }

    pushLog(msg);
    pushStatus(AutomateTestStatus::EndOfTest);

    // 推送自动化测试数据
    QString json;
    json << automateTestResult;
    RedisHelper::getInstance().publish(AutoTestDataChannel, json);
}
