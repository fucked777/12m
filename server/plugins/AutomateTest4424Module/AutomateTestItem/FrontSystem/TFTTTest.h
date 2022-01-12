#ifndef TFTTTest_H
#define TFTTTest_H
#include "AutomateTestBase.h"

/*
 * AGC时常测试开始
 */
class AutomateTest;
class TFTTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    TFTTTest(AutomateTest* parent);
    ~TFTTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
};

#endif  // TFTTTest_H
