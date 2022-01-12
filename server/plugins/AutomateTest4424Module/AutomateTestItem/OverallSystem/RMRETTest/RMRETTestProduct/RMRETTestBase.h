#ifndef RMRETTESTBASE_H
#define RMRETTESTBASE_H

#include "AutomateTestBase.h"
#include "IRMRETTestProduct.h"

class RMRETTestBase : public IRMRETTestProduct
{
public:
    RMRETTestBase(AutomateTestBase* autotestbase);

    bool setLoop() override;
    bool setXLReduce() override;
    bool saveResult(const CmdResult cmdResult) override;

protected:
    DeviceID m_hpID; /* 功放设备的ID */
    DeviceID m_ckjd; /* 测控基带的设备ID */
    AutomateTestBase* m_autoTestBase = nullptr;
};

#endif  // RMRETTESTBASE_H
