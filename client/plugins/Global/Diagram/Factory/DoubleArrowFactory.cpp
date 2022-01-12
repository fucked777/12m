#include "DoubleArrowFactory.h"
#include "DoubleArrowItem.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(DoubleArrowFactory, fromElement, toElement)
void DoubleArrowFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(DoubleArrowFactory, func);
    }
}
void DoubleArrowFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(DoubleArrowFactory, func);
    }
}

void DoubleArrowFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(DoubleArrowFactory, func);
    }
}

QString DoubleArrowFactory::typeID() { return "DoubleArrow"; }
GraphInfo DoubleArrowFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new DoubleArrowItem;
    GetValueHelper<DoubleArrowItem> autoValue(item);

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
    info.setItemData = ITEMDATAFUNC_TEMPLATE(DoubleArrowFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(DoubleArrowFactory);
    info.setBorderColorData = BORDERCOLORDATAFUNC_TEMPLATE(DoubleArrowFactory);
    return info;
}
bool DoubleArrowFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const DoubleArrowItem*>(info.item);
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
void DoubleArrowFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    auto item = dynamic_cast<DoubleArrowItem*>(info.item);
    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    item->setArrowColor(color);
    item->setPenColor(color);
}
void DoubleArrowFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    auto item = dynamic_cast<DoubleArrowItem*>(info.item);
    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    item->setArrowColor(color);
    item->setPenColor(color);
}

void DoubleArrowFactory::defaultSetBorderColorData(const GraphInfo& info, const QVariant&)
{
    auto item = dynamic_cast<DoubleArrowItem*>(info.item);
    Q_ASSERT(item != nullptr);
}
