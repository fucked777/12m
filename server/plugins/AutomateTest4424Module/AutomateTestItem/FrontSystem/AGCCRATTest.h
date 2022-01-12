#ifndef AGCCRATTest_H
#define AGCCRATTest_H
#include "AutomateTestBase.h"

/*
 * AGC控制范围、精度测试
 */
class AutomateTest;
class AGCCRATTest : public AutomateTestBase
{
    Q_OBJECT
public:
    AGCCRATTest(AutomateTest* parent);
    ~AGCCRATTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
};

#endif  // AGCCRATTest_H
