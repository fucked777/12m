#ifndef SRETTESTSYTHWXPRODUCT_H
#define SRETTESTSYTHWXPRODUCT_H

#include "SRETTestBase.h"

class SRETTestSYTHWXProduct : public SRETTestBase
{
public:
    SRETTestSYTHWXProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SRETTESTSYTHWXPRODUCT_H
