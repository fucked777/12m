#ifndef SCTTTESTBASE_H
#define SCTTTESTBASE_H

#include "AutomateTestBase.h"
#include "ISCTTTestProduct.h"

class SCTTTestBase : public ISCTTTestProduct
{
public:
    SCTTTestBase(AutomateTestBase* autotestbase);

    bool setLoop() override;
    bool setXLReduce() override;
    bool saveResult(const CmdResult cmdResult) override;

protected:
    DeviceID m_hpID; /* 功放设备的ID */
    DeviceID m_ckjd; /* 测控基带的设备ID */
    AutomateTestBase* m_autoTestBase = nullptr;
};

#endif  // SCTTTESTBASE_H
