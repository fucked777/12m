#ifndef SCTTTESTSKUO2PRODUCT_H
#define SCTTTESTSKUO2PRODUCT_H

#include "SCTTTestBase.h"

class SCTTTestSkuo2Product : public SCTTTestBase
{
public:
    SCTTTestSkuo2Product(AutomateTestBase* autotestbase);

    bool setBaseCmd() override;
    bool startTest(CmdResult& cmdResult) override;
};

#endif  // SCTTTESTSKUO2PRODUCT_H
