#ifndef AUTOMATETESTFACTORYREGISTERTEMPLATE_H
#define AUTOMATETESTFACTORYREGISTERTEMPLATE_H
#include "AutomateTestFactory.h"

/* 此文件定义了一个特殊的静态模板
 * 此模板功能是利用static的特性在程序加载的时候可以
 * 自动的注册相关的图元
 */

#define REGISTER_PRIVAE_CLASS_TEMPLATE(__ClassName, __WorkMode, __CreateFunc)                                                                        \
    class __ClassName##__WorkMode##Private__                                                                                                         \
    {                                                                                                                                                \
    public:                                                                                                                                          \
        __ClassName##__WorkMode##Private__(const QString& typeID, SystemWorkMode workMode, CreateTestContext func)                                   \
        {                                                                                                                                            \
            AutomateTestFactory::registerAutomateTestItemType(typeID, workMode, func);                                                               \
        }                                                                                                                                            \
        static __ClassName##__WorkMode##Private__ g##__ClassName##Var;                                                                               \
    };                                                                                                                                               \
    __ClassName##__WorkMode##Private__ __ClassName##__WorkMode##Private__::g##__ClassName##Var(__ClassName::typeID(), SystemWorkMode::__WorkMode,    \
                                                                                               &__ClassName::__CreateFunc);

#endif  // AUTOMATETESTFACTORYREGISTERTEMPLATE_H
