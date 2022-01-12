#include "SRETTestSkuo2Product.h"

SRETTestSkuo2Product::SRETTestSkuo2Product(AutomateTestBase* autotestbase)
    : SRETTestBase(autotestbase)
{
}

bool SRETTestSkuo2Product::setBaseCmd()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /* 设置测控基带目标1
     * 接收单元遥测支路伪码码率为5115kc
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_TIRU_AUTOTESTRMRET_SpreadCode", packCommand, m_ckjd);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /* 设置测控基带目标1
     * Gold码参数如下：多项式A：197H，初相A：35AH，多项式B：81H，初相B：248H
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_SSCU_AUTOTEST_PNCodeMulti", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /* 设置测控基带目标1
     * 遥测码率1kbps，码型NRZ-L，
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_TU_AUTOTESTRMRET_Code", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /* 设置测控基带中频调制单元目标1遥测支路加调，其它目标去调，控制测控基带输出单载波
     * 20210830 wp?? 没有遥测支路 这里先都加调
     *
     * 先全部去调 然后单独的将通道1加调
     */
    singleCmdHelper.packSingleCommand("StepCKJD_Skuo2_JTKZ_AUTOTEST", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_IFCM_AUTOTESTRMRET_Add", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool SRETTestSkuo2Product::startTest(CmdResult& cmdResult)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;    /* 开始测试 */
    replaceParamMap["RotatDirect"] = 1; /* 旋向 */

    auto statPoint = m_autoTestBase->m_testPlanItem.testParameterMap.value("StatPoint").rawValue.toInt();
    if (statPoint <= 0)
    {
        m_autoTestBase->pushLog("测试错误:当前统计及点数是非法值", true);
        return false;
    }
    replaceParamMap["StatPoint"] = statPoint;
    replaceParamMap["ChannelNumb"] = 1; /* 选通道1 */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    singleCmdHelper.packSingleCommand(QString("StepCKJD_SKuo2_AUTOTESTRMRET"), packCommand, m_ckjd, replaceParamMap);
    auto execStatus = m_autoTestBase->waitExecResponceCondition(
        cmdResult, packCommand, 3, 1200, statPoint, [](CmdResult& cmdResult, QString& errMsg, const QVariant& arg) {
            if (!cmdResult.m_paramdataMap.contains("StatCount"))
            {
                errMsg = "上报结果无已统计点数的数据";
                return AutomateTestBase::ConditionResult::Error;
            }
            auto statCount = cmdResult.m_paramdataMap.value("StatCount").toInt();
            return (statCount >= arg.toInt() ? AutomateTestBase::ConditionResult::OK : AutomateTestBase::ConditionResult::Wait);
        });
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}
