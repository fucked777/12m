#include "SCTTTestSYTHGMLProduct.h"

SCTTTestSYTHGMLProduct::SCTTTestSYTHGMLProduct(AutomateTestBase* autotestbase)
    : SCTTTestBase(autotestbase)
{
}

bool SCTTTestSYTHGMLProduct::setBaseCmd()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /*
     *设置测控基带下行数传单元设置信息速率与帧频1.04 码型NRZ-L  译码方式为Turbo（1/3）
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SYTHGML_TU_AUTOTEST_Code", packCommand, m_ckjd);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /*
     *设置测控基带数传模拟源单元设置信息速率与帧频1.04 码型NRZ-L  译码方式为Turbo（1/3）
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SYTHGML_TAS_AUTOTEST_Code", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    /*中频调制单元加调*/
    singleCmdHelper.packSingleCommand("StepCKJD_SYTHGML_JTKZ_AUTOTEST", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool SCTTTestSYTHGMLProduct::startTest(CmdResult& cmdResult)
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
    singleCmdHelper.packSingleCommand(QString("StepCKJD_SYTHGML_AUTOTESTSCTT"), packCommand, m_ckjd, replaceParamMap);
    auto execStatus = m_autoTestBase->waitExecResponceData(cmdResult, packCommand, 3, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}
