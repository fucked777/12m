#ifndef SRETTESTSYTHGMLPRODUCT_H
#define SRETTESTSYTHGMLPRODUCT_H

#include "SRETTestBase.h"

class SRETTestSYTHGMLProduct : public SRETTestBase
{
public:
    SRETTestSYTHGMLProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SRETTESTSYTHGMLPRODUCT_H
