#ifndef LINEFACTORY_H
#define LINEFACTORY_H
#include "GraphFactory.h"

class LineFactory
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

#endif  // LINEFACTORY_H
