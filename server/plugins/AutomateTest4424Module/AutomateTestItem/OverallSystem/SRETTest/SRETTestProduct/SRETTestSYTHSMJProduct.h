#ifndef SRETTESTSYTHSMJPRODUCT_H
#define SRETTESTSYTHSMJPRODUCT_H

#include "SRETTestBase.h"

class SRETTestSYTHSMJProduct : public SRETTestBase
{
public:
    SRETTestSYTHSMJProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SRETTESTSYTHSMJPRODUCT_H
