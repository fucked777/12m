#ifndef SRETTESTFACTORY_H
#define SRETTESTFACTORY_H

#include "AutomateTestBase.h"
#include "ISRETTestProduct.h"

class SRETTestFactory
{
public:
    SRETTestFactory();

    ISRETTestProduct* createProduct(const SystemWorkMode workmod, AutomateTestBase* autotestimpl);
};

#endif  // SRETTESTFACTORY_H
