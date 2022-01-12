#ifndef SRETTESTBASE_H
#define SRETTESTBASE_H

#include "AutomateTestBase.h"
#include "ISRETTestProduct.h"

class SRETTestBase : public ISRETTestProduct
{
public:
    SRETTestBase(AutomateTestBase* autotestbase);

    bool setLoop() override;
    bool setXLReduce() override;
    bool saveResult(const CmdResult cmdResult) override;

protected:
    DeviceID m_hpID; /* 功放设备的ID */
    DeviceID m_ckjd; /* 测控基带的设备ID */
    AutomateTestBase* m_autoTestBase = nullptr;
};

#endif  // SRETTESTBASE_H
