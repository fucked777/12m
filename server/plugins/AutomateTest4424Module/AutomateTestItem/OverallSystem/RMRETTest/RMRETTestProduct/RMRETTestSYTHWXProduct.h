#ifndef RMRETTESTSYTHWXPRODUCT_H
#define RMRETTESTSYTHWXPRODUCT_H

#include "RMRETTestBase.h"

class RMRETTestSYTHWXProduct : public RMRETTestBase
{
public:
    RMRETTestSYTHWXProduct(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // RMRETTESTSYTHWXPRODUCT_H
