#ifndef RMSETTEST_H
#define RMSETTEST_H
#include "AutomateTestBase.h"
#include "ProtocolXmlTypeDefine.h"

/*
 * 测距系统误差测试
 */
class SingleCommandHelper;
class AutomateTest;
class RMSETTest : public AutomateTestBase
{
    Q_OBJECT
public:
    RMSETTest(AutomateTest* parent);
    ~RMSETTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    virtual bool pretreatment() override;                                  /* 参数预处理 */
    virtual bool linkConfiguration() override;                             /* 设置环路 */
    virtual bool parameterSetting() override;                              /* 参数设置 */
    virtual bool testing(CmdResult& /*cmdResult*/) override;               /* 开始测试 */
    virtual bool resultOperation(const CmdResult& /*cmdResult*/) override; /* 结果处理 */
    virtual void endOfTest() override; /* 结束测试(清理) 这个不能太耗时 不用等结果直接发指令就行 */

    bool testOne(SingleCommandHelper& singleCmdHelper, QList<double>& result, const QVariantMap& replaceParamMap, int statPoint);

private:
    DeviceID m_hpID;                /* 功放设备的ID */
    DeviceID m_ckjd;                /* 测控基带的设备ID */
    QList<double> m_testResultList; /* 测试结果列表 */
    int m_timerLoopCount = 0;       /* 测试次数 */
};

#endif  // RMSETTEST_H
