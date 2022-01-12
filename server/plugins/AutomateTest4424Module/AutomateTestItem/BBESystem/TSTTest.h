#ifndef TSTTest_H
#define TSTTest_H
#include "AutomateTestBase.h"

/*
 * 调制度测试
 */
class AutomateTest;
class TSTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    TSTTest(AutomateTest* parent);
    ~TSTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
};

#endif  // TSTTest_H
