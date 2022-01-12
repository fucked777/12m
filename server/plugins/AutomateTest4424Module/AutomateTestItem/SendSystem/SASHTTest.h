#ifndef SASHTTest_H
#define SASHTTest_H
#include "AutomateTestBase.h"

/*
 * 杂散及二次谐波测试开始
 */
class AutomateTest;
class SASHTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    SASHTTest(AutomateTest* parent);
    ~SASHTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);
    //打通信号环路
    void openLoop();

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // SASHTTest_H
