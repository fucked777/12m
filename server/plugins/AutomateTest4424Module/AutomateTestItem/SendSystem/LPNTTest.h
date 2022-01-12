#ifndef LPNTTest_H
#define LPNTTest_H
#include "AutomateTestBase.h"

/*
 * 相位噪声测试开始
 */
class AutomateTest;
class LPNTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    LPNTTest(AutomateTest* parent);
    ~LPNTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);
    //打通信号环路
    void openLoop();

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // LPNTTest_H
