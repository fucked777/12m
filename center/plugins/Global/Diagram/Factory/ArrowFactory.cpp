#include "ArrowFactory.h"
#include "ArrowItem.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "ItemBase.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(ArrowFactory, fromElement, toElement)
void ArrowFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(ArrowFactory, func);
    }
}
void ArrowFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(ArrowFactory, func);
    }
}

void ArrowFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(ArrowFactory, func);
    }
}

QString ArrowFactory::typeID() { return "Arrow"; }
GraphInfo ArrowFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new ArrowItem;
    GetValueHelper<ArrowItem> autoValue(item);
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

    /* 角度 */
    GraphFactoryUtility::rotationFromElement(item, element);

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(ArrowFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(ArrowFactory);
    info.setBorderColorData = BORDERCOLORDATAFUNC_TEMPLATE(ArrowFactory);
    return info;
}
bool ArrowFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const ArrowItem*>(info.item);
    Q_ASSERT(item != nullptr);
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

    /* 解析角度 */
    GraphFactoryUtility::rotationToElement(item, element);

    return true;
}
void ArrowFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<ArrowItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}
void ArrowFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<ArrowItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setBrushColor(color);
    // info.item->setPenColor(color);
}

void ArrowFactory::defaultSetBorderColorData(const GraphInfo& info, const QVariant&) { Q_ASSERT(info.item != nullptr); }
