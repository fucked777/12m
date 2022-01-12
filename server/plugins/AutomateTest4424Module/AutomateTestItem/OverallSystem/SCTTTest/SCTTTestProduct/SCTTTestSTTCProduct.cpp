#include "SCTTTestSTTCProduct.h"

SCTTTestSTTCProduct::SCTTTestSTTCProduct(AutomateTestBase* autotestbase)
    : SCTTTestBase(autotestbase)
{
}

bool SCTTTestSTTCProduct::setBaseCmd()
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

bool SCTTTestSTTCProduct::startTest(CmdResult& cmdResult)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;    /* 开始测试 */
    replaceParamMap["RotatDirect"] = 1; /* 旋向 */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    singleCmdHelper.packSingleCommand(QString("StepCKJD_STTC_AUTOTESTSCTT"), packCommand, m_ckjd, replaceParamMap);
    auto execStatus = m_autoTestBase->waitExecResponceData(cmdResult, packCommand, 3, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}
