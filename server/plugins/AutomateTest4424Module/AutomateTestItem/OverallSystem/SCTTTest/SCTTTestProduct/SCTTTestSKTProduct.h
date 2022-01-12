#ifndef SCTTTESTSKTPRODUCT_H
#define SCTTTESTSKTPRODUCT_H

#include "SCTTTestBase.h"

class SCTTTestSKTProduct : public SCTTTestBase
{
public:
    SCTTTestSKTProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SCTTTESTSKTPRODUCT_H
