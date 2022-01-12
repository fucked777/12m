#ifndef RMRETTEST_H
#define RMRETTEST_H
#include "AutomateTestBase.h"
#include "ProtocolXmlTypeDefine.h"

/*
 * 测距随机误差测试
 */
class AutomateTest;
class RMRETTest : public AutomateTestBase
{
    Q_OBJECT
public:
    RMRETTest(AutomateTest* parent);
    ~RMRETTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
    void stopImpl() override;
};

#endif  // RMRETTEST_H
