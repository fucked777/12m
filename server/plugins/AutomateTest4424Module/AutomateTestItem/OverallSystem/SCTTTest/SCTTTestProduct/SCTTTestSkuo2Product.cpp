#include "SCTTTestSkuo2Product.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "BusinessMacroCommon.h"
#include "ControlFlowHandler.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlAutomateTest.h"

SCTTTestSkuo2Product::SCTTTestSkuo2Product(AutomateTestBase* autotestbase)
    : SCTTTestBase(autotestbase)
{
}

bool SCTTTestSkuo2Product::setBaseCmd()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /* 设置测控基带目标1
     * 接收单元遥测支路伪码码率为10230kc
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_TIRU_AUTOTEST_SpreadCode", packCommand, m_ckjd);
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
     * 遥测码率2kbps，码型NRZ-L，
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_TU_AUTOTEST_Code", packCommand, m_ckjd);
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
    singleCmdHelper.packSingleCommand("StepCKJD_SKuo2_IFCM_AUTOTEST_Add", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool SCTTTestSkuo2Product::startTest(CmdResult& cmdResult)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 1;    /* 开始测试 */
    replaceParamMap["RotatDirect"] = 1; /* 旋向 */

    /* 1遥测2测距 */
    auto tempChanCategory = m_autoTestBase->m_testPlanItem.testParameterMap.value("ChanCategory").rawValue.toInt();
    replaceParamMap["ChanCategory"] = (tempChanCategory == 1 ? 1 : 2); /* 通道类别 */
    replaceParamMap["ChannelNumb"] = 1;                                /* 选通道1 */

    /*
     * 这里和下链路不一样
     * 这里错误直接就返回错误就行了
     */
    singleCmdHelper.packSingleCommand(QString("StepCKJD_SKuo2_AUTOTESTSCTT"), packCommand, m_ckjd, replaceParamMap);
    auto execStatus = m_autoTestBase->waitExecResponceData(cmdResult, packCommand, 3, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}
