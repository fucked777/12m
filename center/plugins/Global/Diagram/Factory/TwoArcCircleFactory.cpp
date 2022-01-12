#include "TwoArcCircleFactory.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "TwoArcCircleItem.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(TwoArcCircleFactory, fromElement, toElement)
void TwoArcCircleFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(TwoArcCircleFactory, func);
    }
}
void TwoArcCircleFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(TwoArcCircleFactory, func);
    }
}

void TwoArcCircleFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(TwoArcCircleFactory, func);
    }
}

QString TwoArcCircleFactory::typeID() { return "TwoArcCircle"; }
GraphInfo TwoArcCircleFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new TwoArcCircleItem;
    GetValueHelper<TwoArcCircleItem> autoValue(item);

    /* id */
    if (!GraphFactoryUtility::idFromElement(info, item, typeID(), element))
    {
        return {};
    }
    /* 全局属性 */
    GraphFactoryUtility::globalFromElement(item, cGlobal, element);

    /* 文本 */
    if (!GraphFactoryUtility::textFromElement(item, element))
    {
        return {};
    }

    /* 位置 */
    if (!GraphFactoryUtility::posFromElement(item, element))
    {
        return {};
    }
    /* 半径 */
    if (!GraphFactoryUtility::radiusFromElement(item, element))
    {
        return {};
    }
    /* 角度 */
    GraphFactoryUtility::rotationFromElement(item, element);

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(TwoArcCircleFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(TwoArcCircleFactory);
    return info;
}
bool TwoArcCircleFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const TwoArcCircleItem*>(info.item);
    /* id */
    if (!GraphFactoryUtility::idToElement(info, element))
    {
        return {};
    }

    /* 文本 */
    if (!GraphFactoryUtility::textToElement(item, cGlobal, element))
    {
        return {};
    }
    /* 全局属性 */
    GraphFactoryUtility::globalToElement(item, cGlobal, element);

    /* 位置 */
    GraphFactoryUtility::posToElement(item, element);
    /* 半径 */
    GraphFactoryUtility::radiusToElement(item, element);
    /* 角度 */
    GraphFactoryUtility::rotationToElement(item, element);

    return true;
}
void TwoArcCircleFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}
void TwoArcCircleFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}

void TwoArcCircleFactory::defaultSetBorderColorData(const GraphInfo& /*info*/, const QVariant&) {}
