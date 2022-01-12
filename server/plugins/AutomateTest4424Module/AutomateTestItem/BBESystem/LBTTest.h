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
    void startImpl() override;
};

#endif  // LBTTest_H
