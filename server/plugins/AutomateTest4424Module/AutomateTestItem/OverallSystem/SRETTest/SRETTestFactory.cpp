#include "SRETTestFactory.h"

SRETTestFactory::SRETTestFactory() {}

ISRETTestProduct* SRETTestFactory::createProduct(const SystemWorkMode workmod, AutomateTestBase* autotestimpl)
{
    //    if (workmod == STTC)
    //    {
    //        return new RMRETTestSTTCProduct(autotestimpl);
    //    }
    //    else if (workmod == Skuo2)
    //    {
    //        return new RMRETTestSkuo2Product(autotestimpl);
    //    }
    //    else if (workmod == SYTHSMJ)
    //    {
    //        return new RMRETTestSYTHSMJProduct(autotestimpl);
    //    }
    //    else if (workmod == SYTHWX)
    //    {
    //        return new RMRETTestSYTHWXProduct(autotestimpl);
    //    }
    //    else if (workmod == SYTHGML)
    //    {
    //        return new RMRETTestSYTHGMLProduct(autotestimpl);
    //    }

    return nullptr;
}
