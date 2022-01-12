#ifndef STTTest_H
#define STTTest_H
#include "AutomateTestBase.h"

/*
 * 杂散测试开始
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
    //打通信号环路
    void openLoop();

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // STTTest_H
