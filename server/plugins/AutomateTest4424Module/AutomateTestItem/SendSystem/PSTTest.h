#ifndef PSTTest_H
#define PSTTest_H
#include "AutomateTestBase.h"

/*
 * 功率稳定性测试开始
 */
class AutomateTest;
class PSTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    PSTTest(AutomateTest* parent);
    ~PSTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

    //打通信号环路
    void openLoop();

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // PSTTest_H
