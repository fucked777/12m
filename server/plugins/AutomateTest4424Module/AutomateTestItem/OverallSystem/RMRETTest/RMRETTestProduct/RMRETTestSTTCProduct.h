#ifndef RMRETTESTSTTCPRODUCT_H
#define RMRETTESTSTTCPRODUCT_H

#include "RMRETTestBase.h"

class RMRETTestSTTCProduct : public RMRETTestBase
{
public:
    RMRETTestSTTCProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // RMRETTESTSTTCPRODUCT_H
