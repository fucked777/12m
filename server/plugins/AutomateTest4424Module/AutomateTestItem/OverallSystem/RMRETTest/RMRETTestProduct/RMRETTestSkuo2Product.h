#ifndef RMRETTESTSKUO2PRODUCT_H
#define RMRETTESTSKUO2PRODUCT_H

#include "RMRETTestBase.h"

class RMRETTestSkuo2Product : public RMRETTestBase
{
public:
    RMRETTestSkuo2Product(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // RMRETTESTSKUO2PRODUCT_H
