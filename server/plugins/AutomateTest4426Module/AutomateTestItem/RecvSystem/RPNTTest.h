#ifndef RPNTTest_H
#define RPNTTest_H
#include "AutomateTestBase.h"

/*
 * G/T值测试
 */
class AutomateTest;
class RPNTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    RPNTTest(AutomateTest* parent);
    ~RPNTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    virtual bool linkConfiguration() override;                         /* 设置环路 */
    virtual bool parameterSetting() override;                          /* 参数设置 */
    virtual bool testing(CmdResult& cmdResult) override;               /* 开始测试 */
    virtual bool resultOperation(const CmdResult& cmdResult) override; /* 结果处理 */
    virtual void endOfTest() override; /* 结束测试(清理) 这个不能太耗时 不用等结果直接发指令就行 */
private:
    QList<CmdResult> m_cmdResultList;
};

#endif  // RPNTTest_H
