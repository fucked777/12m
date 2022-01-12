#ifndef CCTTEST_H
#define CCTTEST_H
#include "AutomateTestBase.h"
#include "ProtocolXmlTypeDefine.h"

/*
 * 收敛特性测试
 */
class AutomateTest;
class CCTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    CCTTest(AutomateTest* parent);
    ~CCTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    virtual bool pretreatment() override;                              /* 参数预处理 */
    virtual bool linkConfiguration() override;                         /* 设置环路 */
    virtual bool parameterSetting() override;                          /* 参数设置 */
    virtual bool testing(CmdResult& cmdResult) override;               /* 开始测试 */
    virtual bool resultOperation(const CmdResult& cmdResult) override; /* 结果处理 */
    virtual void endOfTest() override; /* 结束测试(清理) 这个不能太耗时 不用等结果直接发指令就行 */

private:
    DeviceID m_deviceACUMaster;
};

#endif  // CCTTEST_H
