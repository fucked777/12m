#ifndef SCTTTESTSYTHSMJPRODUCT_H
#define SCTTTESTSYTHSMJPRODUCT_H

#include "SCTTTestBase.h"

class SCTTTestSYTHSMJProduct : public SCTTTestBase
{
public:
    SCTTTestSYTHSMJProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SCTTTESTSYTHSMJPRODUCT_H
