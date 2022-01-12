#include "ArcCrossCircleFactory.h"
#include "ArcCrossCircleItem.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(ArcCrossCircleFactory, fromElement, toElement)
void ArcCrossCircleFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(ArcCrossCircleFactory, func);
    }
}
void ArcCrossCircleFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(ArcCrossCircleFactory, func);
    }
}

void ArcCrossCircleFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(ArcCrossCircleFactory, func);
    }
}

QString ArcCrossCircleFactory::typeID() { return "ArcCrossCircle"; }
GraphInfo ArcCrossCircleFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new ArcCrossCircleItem;
    GetValueHelper<ArcCrossCircleItem> autoValue(item);

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

    /* 类型这个是这个开关特有的,默认就是半圆 */
    auto displayType = element.attribute("displayType").trimmed();
    if (!displayType.isEmpty())
    {
        auto upper = displayType.trimmed().toUpper();
        if (upper == "ARC")
        {
            item->setDisplayType(ArcCrossCircleItem::DisplayType::Arc);
        }
        else if (upper == "CROSS")
        {
            item->setDisplayType(ArcCrossCircleItem::DisplayType::Cross);
        }
        else
        {
            return {};
        }
    }

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(ArcCrossCircleFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(ArcCrossCircleFactory);
    info.setBorderColorData = BORDERCOLORDATAFUNC_TEMPLATE(ArcCrossCircleFactory);
    return info;
}
bool ArcCrossCircleFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const ArcCrossCircleItem*>(info.item);
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

    /* 类型这个是这个开关特有的,默认就是半圆 */
    auto tempType = item->displayType();
    if (tempType == ArcCrossCircleItem::DisplayType::Arc)
    {
        element.setAttribute("displayType", "Arc");
    }
    else
    {
        element.setAttribute("displayType", "Cross");
    }

    return true;
}
void ArcCrossCircleFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}
void ArcCrossCircleFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}

void ArcCrossCircleFactory::defaultSetBorderColorData(const GraphInfo& info, const QVariant&) { Q_ASSERT(info.item != nullptr); }
