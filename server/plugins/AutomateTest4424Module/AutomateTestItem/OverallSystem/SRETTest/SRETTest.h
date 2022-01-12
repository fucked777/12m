#ifndef SRETTEST_H
#define SRETTEST_H
#include "AutomateTestBase.h"
#include "ProtocolXmlTypeDefine.h"

/*
 * 测速随机误差测试
 */
class AutomateTest;
class SRETTest : public AutomateTestBase
{
    Q_OBJECT
public:
    SRETTest(AutomateTest* parent);
    ~SRETTest() override;
    static QString typeID();
    static AutomateTestBase* createTestItem(AutomateTest* parent);

protected:
    void startImpl() override;
    void stopImpl() override;

private:
    DeviceID m_hpID; /* 功放设备的ID */
    DeviceID m_ckjd; /* 测控基带的设备ID */
};

#endif  // SRETTEST_H
