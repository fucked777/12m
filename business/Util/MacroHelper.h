#ifndef MACROHELPER_H
#define MACROHELPER_H

#define DIS_MOVE(Type)                                                                                                                               \
    Type(Type&&) = delete;                                                                                                                           \
    Type& operator=(Type&&) = delete;

#define DIS_COPY(Type)                                                                                                                               \
    Type(const Type&) = delete;                                                                                                                      \
    Type& operator=(const Type&) = delete;

#define DIS_COPY_MOVE(Type)                                                                                                                          \
    DIS_MOVE(Type)                                                                                                                                   \
    DIS_COPY(Type)

#define EnumArray(x)   g##x##Str
#define EnumStr1(x, v) g##x##Str[static_cast<int>(v)]
#define EnumStr2(x, v) g##x##Str[static_cast<int>(x::v)]

#define EnumNum1(t, v)    static_cast<t>(v)
#define EnumNum2(t, x, v) static_cast<t>(x::v)
#define EnumNum3(v)       EnumNum1(int, v)
#define EnumNum4(x, v)    EnumNum2(int, x, v)

#define STR1(R)    #R
#define MACRSTR(R) STR1(R)

/* 当前的模块名称 */
#ifndef MODULE_NAME
#define MODULE_NAME UnknownModule
#endif

#endif  // MACROHELPER_H
