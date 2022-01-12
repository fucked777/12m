#include "CircleFactory.h"
#include "CircleItem.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "Utility.h"
#include <QDomElement>

REGISTER_PRIVAE_CLASS_TEMPLATE(CircleFactory, fromElement, toElement)
void CircleFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(CircleFactory, func);
    }
}
void CircleFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(CircleFactory, func);
    }
}

void CircleFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(CircleFactory, func);
    }
}

QString CircleFactory::typeID() { return "Circle"; }
GraphInfo CircleFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new CircleItem;
    GetValueHelper<CircleItem> autoValue(item);

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

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(CircleFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(CircleFactory);
    info.setBorderColorData = BORDERCOLORDATAFUNC_TEMPLATE(CircleFactory);
    return info;
}
bool CircleFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const CircleItem*>(info.item);
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

    return true;
}
void CircleFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<ArrowItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}
void CircleFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<ArrowItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}

void CircleFactory::defaultSetBorderColorData(const GraphInfo& info, const QVariant&) { Q_ASSERT(info.item != nullptr); }
