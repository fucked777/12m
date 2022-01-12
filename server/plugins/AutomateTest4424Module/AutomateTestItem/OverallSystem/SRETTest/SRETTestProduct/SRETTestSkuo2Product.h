#ifndef SRETTESTSKUO2PRODUCT_H
#define SRETTESTSKUO2PRODUCT_H

#include "SRETTestBase.h"

class SRETTestSkuo2Product : public SRETTestBase
{
public:
    SRETTestSkuo2Product(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SRETTESTSKUO2PRODUCT_H
