#ifndef FRTTest_H
#define FRTTest_H
#include "AutomateTestBase.h"

/*
 * 频率响应测试开始
 */
class AutomateTest;
class FRTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    FRTTest(AutomateTest* parent);
    ~FRTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);
    //打通信号环路
    void openLoop();

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // FRTTest_H
