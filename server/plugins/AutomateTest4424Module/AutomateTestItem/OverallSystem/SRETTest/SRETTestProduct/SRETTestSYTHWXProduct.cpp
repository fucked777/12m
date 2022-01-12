#include "SRETTestSYTHWXProduct.h"

SRETTestSYTHWXProduct::SRETTestSYTHWXProduct(AutomateTestBase* autotestbase)
    : SRETTestBase(autotestbase)
{
}

bool SRETTestSYTHWXProduct::setBaseCmd()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    /*
     *设置测控基带下行数传单元设置信息速率与帧频1.04 码型NRZ-L  译码方式为Turbo（1/3）
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SYTHWX_TU_AUTOTEST_Code", packCommand, m_ckjd);
    auto execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }
    /*
     *设置测控基带数传模拟源单元设置信息速率与帧频1.04 码型NRZ-L  译码方式为Turbo（1/3）
     */
    singleCmdHelper.packSingleCommand("StepCKJD_SYTHWX_TAS_AUTOTEST_Code", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    /*中频调制单元加调*/
    singleCmdHelper.packSingleCommand("StepCKJD_SYTHWX_JTKZ_AUTOTEST", packCommand, m_ckjd);
    execStatus = m_autoTestBase->waitExecSuccess(packCommand, 40);
    if (execStatus != AutomateTestBase::ExecStatus::Success)
    {
        return false;
    }

    return true;
}

bool SRETTestSYTHWXProduct::startTest(CmdResult& cmdResult)
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

    singleCmdHelper.packSingleCommand(QString("StepCKJD_SYTHWX_AUTOTESTRMRET"), packCommand, m_ckjd, replaceParamMap);
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
