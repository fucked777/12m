#ifndef STTTest_H
#define STTTest_H
#include "AutomateTestBase.h"

/*
 * G/T值测试
 */
class AutomateTest;
class STTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    STTTest(AutomateTest* parent);
    ~STTTest() override;
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

#endif  // STTTest_H
