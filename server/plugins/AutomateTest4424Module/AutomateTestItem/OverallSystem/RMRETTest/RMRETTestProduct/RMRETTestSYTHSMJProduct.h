#ifndef RMRETTESTSYTHSMJPRODUCT_H
#define RMRETTESTSYTHSMJPRODUCT_H

#include "RMRETTestBase.h"

class RMRETTestSYTHSMJProduct : public RMRETTestBase
{
public:
    RMRETTestSYTHSMJProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // RMRETTESTSYTHSMJPRODUCT_H
