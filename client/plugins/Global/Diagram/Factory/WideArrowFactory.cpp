#include "WideArrowFactory.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "ItemBase.h"
#include "Utility.h"
#include "WideArrowItem.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(WideArrowFactory, fromElement, toElement)
void WideArrowFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(WideArrowFactory, func);
    }
}
void WideArrowFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(WideArrowFactory, func);
    }
}

void WideArrowFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(WideArrowFactory, func);
    }
}

QString WideArrowFactory::typeID() { return "WideArrow"; }
GraphInfo WideArrowFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new WideArrowItem;
    GetValueHelper<WideArrowItem> autoValue(item);

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
    if (!GraphFactoryUtility::wideArrowSizeFromElement(item, element))
    {
        return {};
    }

    /* 解析原点位置 */
    GraphFactoryUtility::sourcePointPositionFromElement(item, element);
    /* 单双箭头 */
    GraphFactoryUtility::wideArrowDoubleFromElement(item, element);

    /* 旋转角度 */
    GraphFactoryUtility::rotationFromElement(item, element);

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(WideArrowFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(WideArrowFactory);
    return info;
}
bool WideArrowFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const WideArrowItem*>(info.item);
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
    GraphFactoryUtility::wideArrowSizeToElement(item, element);

    /* 解析原点位置 */
    GraphFactoryUtility::sourcePointPositionToElement(item, element);
    /* 单双箭头 */
    GraphFactoryUtility::wideArrowDoubleToElement(item, element);
    /* 旋转角度 */
    GraphFactoryUtility::rotationToElement(item, element);
    return true;
}
void WideArrowFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}
void WideArrowFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}

void WideArrowFactory::defaultSetBorderColorData(const GraphInfo& /*info*/, const QVariant&) {}
