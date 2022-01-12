#include "RectFactory.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "ItemBase.h"
#include "RectItem.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(RectFactory, fromElement, toElement)
void RectFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(RectFactory, func);
    }
}
void RectFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(RectFactory, func);
    }
}

void RectFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(RectFactory, func);
    }
}

QString RectFactory::typeID() { return "Rectangle"; }
GraphInfo RectFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new RectItem;
    GetValueHelper<RectItem> autoValue(item);

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
    /* 大小 */
    if (!GraphFactoryUtility::sizeFromElement(item, element))
    {
        return {};
    }

    /* 解析原点位置 */
    GraphFactoryUtility::sourcePointPositionFromElement(item, element);

    /* 旋转角度 */
    GraphFactoryUtility::rotationFromElement(item, element);

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(RectFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(RectFactory);
    info.setBorderColorData = BORDERCOLORDATAFUNC_TEMPLATE(RectFactory);
    return info;
}
bool RectFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const RectItem*>(info.item);
    /* id */
    if (!GraphFactoryUtility::idToElement(info, element))
    {
        return false;
    }

    /* 文本 */
    if (!GraphFactoryUtility::textToElement(item, cGlobal, element))
    {
        return false;
    }
    /* 全局属性 */
    GraphFactoryUtility::globalToElement(item, cGlobal, element);

    /* 位置 */
    GraphFactoryUtility::posToElement(item, element);
    /* 大小 */
    GraphFactoryUtility::sizeToElement(item, element);

    /* 解析原点位置 */
    GraphFactoryUtility::sourcePointPositionToElement(item, element);
    /* 旋转角度 */
    GraphFactoryUtility::rotationToElement(item, element);
    return true;
}
void RectFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}
void RectFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}

void RectFactory::defaultSetBorderColorData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setPenColor(color);
}
