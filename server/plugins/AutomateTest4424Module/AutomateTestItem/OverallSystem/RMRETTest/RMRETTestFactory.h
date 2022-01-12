#ifndef RMRETTESTFACTORY_H
#define RMRETTESTFACTORY_H

#include "AutomateTestBase.h"
#include "IRMRETTestProduct.h"

class RMRETTestFactory
{
public:
    RMRETTestFactory();

    IRMRETTestProduct* createProduct(const SystemWorkMode workmod, AutomateTestBase* autotestimpl);
};

#endif  // RMRETTESTFACTORY_H
