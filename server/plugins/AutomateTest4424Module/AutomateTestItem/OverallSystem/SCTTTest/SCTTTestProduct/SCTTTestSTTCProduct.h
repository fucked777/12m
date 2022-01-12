#ifndef SCTTTESTSTTCPRODUCT_H
#define SCTTTESTSTTCPRODUCT_H

#include "SCTTTestBase.h"

class SCTTTestSTTCProduct : public SCTTTestBase
{
public:
    SCTTTestSTTCProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SCTTTESTSTTCPRODUCT_H
