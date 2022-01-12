#ifndef GTTEST_H
#define GTTEST_H
#include "AutomateTestBase.h"

/*
 * G/T值测试
 */
class AutomateTest;
class GTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    GTTest(AutomateTest* parent);
    ~GTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // GTTEST_H
