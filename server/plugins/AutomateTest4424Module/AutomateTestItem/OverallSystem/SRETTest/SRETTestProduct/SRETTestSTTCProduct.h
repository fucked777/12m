#ifndef SRETTESTSTTCPRODUCT_H
#define SRETTESTSTTCPRODUCT_H

#include "SRETTestBase.h"

class SRETTestSTTCProduct : public SRETTestBase
{
public:
    SRETTestSTTCProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SRETTESTSTTCPRODUCT_H
