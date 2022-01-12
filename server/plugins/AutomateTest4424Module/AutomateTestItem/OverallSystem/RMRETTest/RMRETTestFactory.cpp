#include "RMRETTestFactory.h"
#include "RMRETTestSTTCProduct.h"
#include "RMRETTestSYTHGMLProduct.h"
#include "RMRETTestSYTHSMJProduct.h"
#include "RMRETTestSYTHWXProduct.h"
#include "RMRETTestSkuo2Product.h"

RMRETTestFactory::RMRETTestFactory() {}

IRMRETTestProduct* RMRETTestFactory::createProduct(const SystemWorkMode workmod, AutomateTestBase* autotestimpl)
{
    if (workmod == STTC)
    {
        return new RMRETTestSTTCProduct(autotestimpl);
    }
    else if (workmod == Skuo2)
    {
        return new RMRETTestSkuo2Product(autotestimpl);
    }
    else if (workmod == SYTHSMJ)
    {
        return new RMRETTestSYTHSMJProduct(autotestimpl);
    }
    else if (workmod == SYTHWX)
    {
        return new RMRETTestSYTHWXProduct(autotestimpl);
    }
    else if (workmod == SYTHGML)
    {
        return new RMRETTestSYTHGMLProduct(autotestimpl);
    }

    return nullptr;
}
