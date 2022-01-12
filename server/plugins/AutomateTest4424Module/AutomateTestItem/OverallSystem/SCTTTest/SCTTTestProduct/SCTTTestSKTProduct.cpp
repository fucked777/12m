#include "SCTTTestSKTProduct.h"

SCTTTestSKTProduct::SCTTTestSKTProduct(AutomateTestBase* autotestbase)
    : SCTTTestBase(autotestbase)
{
}

bool SCTTTestSKTProduct::setBaseCmd()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /*
     *设置测控基带中频调制单元加调
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SKT_JTKZ_AUTOTEST", packCommand, m_ckjd);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool SCTTTestSKTProduct::startTest(CmdResult& cmdResult)
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
    singleCmdHelper.packSingleCommand(QString("StepCKJD_SKT_AUTOTESTSCTT"), packCommand, m_ckjd, replaceParamMap);
    auto execStatus = m_autoTestBase->waitExecResponceData(cmdResult, packCommand, 3, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}
