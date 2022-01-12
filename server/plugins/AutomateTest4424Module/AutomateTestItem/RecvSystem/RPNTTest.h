#ifndef RPNTTest_H
#define RPNTTest_H
#include "AutomateTestBase.h"

/*
 * 相位噪声测试开始
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
    //打通信号环路
    void openLoop();

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // RPNTTest_H
