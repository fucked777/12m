#ifndef WIDEARROWFACTORY_H
#define WIDEARROWFACTORY_H
#include "GraphFactory.h"

class WideArrowFactory
{
public:
    static GraphInfo fromElement(const GraphGlobal& cGlobal, const QDomElement&);
    static bool toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement&);
    static QString typeID();

    static void defaultSetItemData(const GraphInfo& info, const QVariant&);
    static void defaultSetGroupData(const GraphInfo& info, const QVariant&);
    static void defaultSetBorderColorData(const GraphInfo& info, const QVariant&);
    static void changeItemDataFunc(SetItemDataFunc func);
    static void changeGroupDataFunc(SetGroupDataFunc func);
    static void changeBorderColorDataFunc(SetBorderColorDataFunc func);
};

#endif  // WIDEARROWFACTORY_H
