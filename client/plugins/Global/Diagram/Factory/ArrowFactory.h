#ifndef ARROWFACTORY_H
#define ARROWFACTORY_H
#include "GraphFactory.h"

class ArrowFactory
{
public:
    static GraphInfo fromElement(const GraphGlobal& cGlobal, const QDomElement&);
    static bool toElement(const GraphInfo&, const GraphGlobal& cGlobal, QDomElement&);
    static QString typeID();

    static void defaultSetItemData(const GraphInfo& info, const QVariant&);
    static void defaultSetGroupData(const GraphInfo& info, const QVariant&);
    static void defaultSetBorderColorData(const GraphInfo& info, const QVariant&);
    static void changeItemDataFunc(SetItemDataFunc func);
    static void changeGroupDataFunc(SetGroupDataFunc func);
    static void changeBorderColorDataFunc(SetBorderColorDataFunc func);
};

#endif  // ARROWFACTORY_H
