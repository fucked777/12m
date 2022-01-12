#ifndef SCTTTEST_H
#define SCTTTEST_H
#include "AutomateTestBase.h"
#include "ProtocolXmlTypeDefine.h"

/*
 * 系统捕获时间测试
 */
class AutomateTest;
class SCTTTest : public AutomateTestBase
{
    Q_OBJECT
public:
    SCTTTest(AutomateTest* parent);
    ~SCTTTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // SCTTTEST_H
