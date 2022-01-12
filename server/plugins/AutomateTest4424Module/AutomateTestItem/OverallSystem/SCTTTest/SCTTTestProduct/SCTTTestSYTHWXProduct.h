#ifndef SCTTTESTSYTHWXPRODUCT_H
#define SCTTTESTSYTHWXPRODUCT_H

#include "SCTTTestBase.h"

class SCTTTestSYTHWXProduct : public SCTTTestBase
{
public:
    SCTTTestSYTHWXProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SCTTTESTSYTHWXPRODUCT_H
