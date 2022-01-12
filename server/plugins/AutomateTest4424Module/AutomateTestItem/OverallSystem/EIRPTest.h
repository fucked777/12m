#ifndef EIRPTEST_H
#define EIRPTEST_H
#include "AutomateTestBase.h"
#include "ProtocolXmlTypeDefine.h"

/*
 * G/T值测试
 */
class AutomateTest;
class EIRPTest : public AutomateTestBase
{
    Q_OBJECT
public:
    EIRPTest(AutomateTest* parent);
    ~EIRPTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
    void stopImpl() override;

private:
    DeviceID m_hpID; /* 功放设备的ID */
};

#endif  // EIRPTEST_H
