#ifndef SCTTTESTFACTORY_H
#define SCTTTESTFACTORY_H

#include "AutomateTestBase.h"
#include "ISCTTTestProduct.h"

class SCTTTestFactory
{
public:
    explicit SCTTTestFactory();

    ISCTTTestProduct* createProduct(const SystemWorkMode workmod, AutomateTestBase* autotestimpl);
};

#endif  // SCTTTESTFACTORY_H
