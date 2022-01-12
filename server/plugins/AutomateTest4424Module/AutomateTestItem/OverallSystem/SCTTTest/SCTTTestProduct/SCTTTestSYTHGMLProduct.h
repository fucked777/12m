#ifndef SCTTTESTSYTHGMLPRODUCT_H
#define SCTTTESTSYTHGMLPRODUCT_H

#include "SCTTTestBase.h"

class SCTTTestSYTHGMLProduct : public SCTTTestBase
{
public:
    SCTTTestSYTHGMLProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SCTTTESTSYTHGMLPRODUCT_H
