#ifndef LBTTest_H
#define LBTTest_H
#include "AutomateTestBase.h"

/*
 * 环路带宽测试
 */
class AutomateTest;
class LBTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    LBTTest(AutomateTest* parent);
    ~LBTTest() override;
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
    DeviceID m_ckjd;
    QList<CmdResult> m_cmdResultList;
};

#endif  // LBTTest_H
