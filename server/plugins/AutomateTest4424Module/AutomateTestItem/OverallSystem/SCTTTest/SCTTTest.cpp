#include "SCTTTest.h"
#include "AutomateTest.h"
#include "AutomateTestFactoryRegisterTemplate.h"
#include "AutomateTestHelper.h"
#include "MessagePublish.h"
#include "SCTTTestFactory.h"
#include "SingleCommandHelper.h"
#include "SystemWorkMode.h"

REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, STTC, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, Skuo2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHSMJ, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHWX, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHGML, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHSMJK2BGZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHSMJK2GZB, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHWXSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SYTHGMLSK2, createTestItem);
REGISTER_PRIVAE_CLASS_TEMPLATE(SCTTTest, SKT, createTestItem);

QString SCTTTest::typeID() { return "3_SCTT"; }
AutomateTestBase* SCTTTest::createTestItem(AutomateTest* parent) { return new SCTTTest(parent); }

SCTTTest::SCTTTest(AutomateTest* parent)
    : AutomateTestBase(parent)
{
}
SCTTTest::~SCTTTest() {}
void SCTTTest::stopImpl()
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
    singleCmdHelper.packSingleCommand(QString("StepCKJD_%1_AUTOTESTSCTT").arg(cmdStepFormatArg), packCommand, ckjdDeviceID, replaceParamMap);
    notWaitExecSuccessBeQuiet(packCommand);
}
void SCTTTest::startImpl()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    pushLog("系统捕获时间测试开始");

    pushStatus(AutomateTestStatus::LinkConfiguration);

    SCTTTestFactory factroy;
    auto test = std::shared_ptr<ISCTTTestProduct>(factroy.createProduct(m_testPlanItem.workMode, this));
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
