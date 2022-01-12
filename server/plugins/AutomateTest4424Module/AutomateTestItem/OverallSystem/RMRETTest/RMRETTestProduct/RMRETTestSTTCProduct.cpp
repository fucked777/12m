#include "RMRETTestSTTCProduct.h"

RMRETTestSTTCProduct::RMRETTestSTTCProduct(AutomateTestBase* autotestbase)
    : RMRETTestBase(autotestbase)
{
}

bool RMRETTestSTTCProduct::setBaseCmd()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /* 基带测距加调
     * 主音调制度0.3rad
     * 次音调制度0.3rad
     */
    singleCmdHelper.packSingleCommand("StepCKJD_STTC_IFCM_AUTOTEST_CJ", packCommand, m_ckjd);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /* 最低侧音8Hz */
    auto cmdStepKey = (m_autoTestBase->isLeftSpin() ? "StepCKJD_STTC_LTRU_AUTOTEST_MinSide" :  //
                           "StepCKJD_STTC_RTRU_AUTOTEST_MinSide");
    singleCmdHelper.packSingleCommand(cmdStepKey, packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool RMRETTestSTTCProduct::startTest(CmdResult& cmdResult)
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

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */

    singleCmdHelper.packSingleCommand(QString("StepCKJD_STTC_AUTOTESTRMRET"), packCommand, m_ckjd, replaceParamMap);
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
