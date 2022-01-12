#include "RMRETTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "MessagePublish.h"
#include "RMRETTestFactory.h"
#include "SingleCommandHelper.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SYTHGMLSK2, createTestItem);

// 扩跳无此测试项
// REGISTER_PRIVAE_CLASS_TEMPLATE(RMRETTest, SKT, createTestItem);

QString RMRETTest::typeID() { return "4_RMRET"; }
AutomateTestBase* RMRETTest::createTestItem(AutomateTest* parent) { return new RMRETTest(parent); }

RMRETTest::RMRETTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
RMRETTest::~RMRETTest() {}
void RMRETTest::stopImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    DeviceID ckjdDeviceID;
    DeviceID hpDeviceID;
    if (!m_configMacroData.getCKJDDeviceID(m_testPlanItem.workMode, ckjdDeviceID))
    {
        pushLog("获取当前主用测控基带错误", true);
        return;
    }

    if (!m_configMacroData.getSGGFDeviceID(m_testPlanItem.workMode, hpDeviceID))
    {
        pushLog("获取当前主用高功放错误", true);
        return;
    }
    /* 校零变频器关电 */
    AutomateTestHelper::xlbpqPowerOff(singleCmdHelper, this);
    /* 功放关电 */
    AutomateTestHelper::hpPowerOff(singleCmdHelper, this, hpDeviceID);

    /* 停止当前测试 */
    auto cmdStepFormatArg = SystemWorkModeHelper::systemWorkModeToString(m_testPlanItem.workMode);
    QVariantMap replaceParamMap;
    replaceParamMap["TestCtrl"] = 2; /* 停止测试 */
    singleCmdHelper.packSingleCommand(QString("StepCKJD_%1_AUTOTESTRMRET").arg(cmdStepFormatArg), packCommand, ckjdDeviceID, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
void RMRETTest::startImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    pushLog("测距随机误差测试开始");

    pushStatus(AutomateTestStatus::LinkConfiguration);

    RMRETTestFactory factroy;
    auto test = std::shared_ptr<IRMRETTestProduct>(factroy.createProduct(m_testPlanItem.workMode, this));
    if (test == nullptr)
    {
        return;
    }

    /*设置环路*/
    if (!test->setLoop())
    {
        return;
    }

    /*下发基带命令*/
    pushStatus(AutomateTestStatus::ParameterSetting);
    if (!test->setBaseCmd())
    {
        return;
    }

    /*校零衰减设置*/
    if (!test->setXLReduce())
    {
        return;
    }

    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);

    /*测试命令下发*/
    CmdResult cmdResult;
    if (!test->startTest(cmdResult))
    {
        return;
    }
    /*保存结果*/
    if (!test->saveResult(cmdResult))
    {
        return;
    }

    pushStatus(AutomateTestStatus::EndOfTest);
}
