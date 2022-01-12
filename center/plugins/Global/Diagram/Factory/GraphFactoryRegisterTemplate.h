#ifndef GRAPHFACTORYREGISTERTEMPLATE_H
#define GRAPHFACTORYREGISTERTEMPLATE_H
#include "GraphFactory.h"

/* 此文件定义了一个特殊的静态模板
 * 此模板功能是利用static的特性在程序加载的时候可以
 * 自动的注册相关的图元
 */

#define REGISTER_PRIVAE_CLASS_TEMPLATE(__ClassName, __FromName, __ToName)                                                                            \
    class __ClassName##Private__                                                                                                                     \
    {                                                                                                                                                \
    public:                                                                                                                                          \
        __ClassName##Private__(const QString& typeID, GraphFromElenemtFunc from, GraphToElenemtFunc to)                                              \
        {                                                                                                                                            \
            GraphFactory::registerGraphType(typeID, from, to);                                                                                       \
        }                                                                                                                                            \
        static __ClassName##Private__ g##__ClassName##Var;                                                                                           \
        static SetItemDataFunc g_SetItemDataFunc;                                                                                                    \
        static SetGroupDataFunc g_SetGroupDataFunc;                                                                                                  \
        static SetBorderColorDataFunc g_SetBorderColorDataFunc;                                                                                      \
    };                                                                                                                                               \
    __ClassName##Private__ __ClassName##Private__::g##__ClassName##Var(__ClassName::typeID(), &__ClassName::__FromName, &__ClassName::__ToName);     \
    SetItemDataFunc __ClassName##Private__::g_SetItemDataFunc = &__ClassName::defaultSetItemData;                                                    \
    SetGroupDataFunc __ClassName##Private__::g_SetGroupDataFunc = &__ClassName::defaultSetGroupData;                                                 \
    SetBorderColorDataFunc __ClassName##Private__::g_SetBorderColorDataFunc = &__ClassName::defaultSetBorderColorData;

#define CHANGE_ITEMDATAFUNC_TEMPLATE(__ClassName, __ItemDataFunc)   __ClassName##Private__::g_SetItemDataFunc = __ItemDataFunc;
#define CHANGE_GROUPDATAFUNC_TEMPLATE(__ClassName, __GroupDataFunc) __ClassName##Private__::g_SetGroupDataFunc = __GroupDataFunc;
#define CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(__ClassName, __BorderColorDataFunc)                                                                      \
    __ClassName##Private__::g_SetBorderColorDataFunc = __BorderColorDataFunc;

#define ITEMDATAFUNC_TEMPLATE(__ClassName)        __ClassName##Private__::g_SetItemDataFunc;
#define GROUPDATAFUNC_TEMPLATE(__ClassName)       __ClassName##Private__::g_SetGroupDataFunc;
#define BORDERCOLORDATAFUNC_TEMPLATE(__ClassName) __ClassName##Private__::g_SetBorderColorDataFunc;

#endif  // GRAPHFACTORYREGISTERTEMPLATE_H
