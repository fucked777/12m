#ifndef TDDTTest_H
#define TDDTTest_H
#include "AutomateTestBase.h"

/*
 * 遥测解调时延测试开始
 */
class AutomateTest;
class TDDTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    TDDTTest(AutomateTest* parent);
    ~TDDTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
};

#endif  // TDDTTest_H
