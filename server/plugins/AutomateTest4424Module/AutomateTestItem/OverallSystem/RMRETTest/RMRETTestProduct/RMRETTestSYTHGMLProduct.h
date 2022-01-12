#ifndef RMRETTESTSYTHGMLPRODUCT_H
#define RMRETTESTSYTHGMLPRODUCT_H

#include "RMRETTestBase.h"

class RMRETTestSYTHGMLProduct : public RMRETTestBase
{
public:
    RMRETTestSYTHGMLProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // RMRETTESTSYTHGMLPRODUCT_H
