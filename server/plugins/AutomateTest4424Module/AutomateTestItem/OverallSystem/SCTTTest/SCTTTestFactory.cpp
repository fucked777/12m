#include "SCTTTestFactory.h"
#include "SCTTTestSKTProduct.h"
#include "SCTTTestSTTCProduct.h"
#include "SCTTTestSYTHGMLProduct.h"
#include "SCTTTestSYTHSMJProduct.h"
#include "SCTTTestSYTHWXProduct.h"
#include "SCTTTestSkuo2Product.h"

SCTTTestFactory::SCTTTestFactory() {}

ISCTTTestProduct* SCTTTestFactory::createProduct(const SystemWorkMode workmod, AutomateTestBase* autotestimpl)
{
    if (workmod == STTC)
    {
        return new SCTTTestSTTCProduct(autotestimpl);
    }
    else if (workmod == Skuo2)
    {
        return new SCTTTestSkuo2Product(autotestimpl);
    }
    else if (workmod == SYTHSMJ)
    {
        return new SCTTTestSYTHSMJProduct(autotestimpl);
    }
    else if (workmod == SYTHWX)
    {
        return new SCTTTestSYTHWXProduct(autotestimpl);
    }
    else if (workmod == SYTHGML)
    {
        return new SCTTTestSYTHGMLProduct(autotestimpl);
    }
    else if (workmod == SKT)
    {
        return new SCTTTestSKTProduct(autotestimpl);
    }

    return nullptr;
}
