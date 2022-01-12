#include "LineFactory.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "LineItem.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(LineFactory, fromElement, toElement)
void LineFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(LineFactory, func);
    }
}
void LineFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(LineFactory, func);
    }
}

void LineFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(LineFactory, func);
    }
}

QString LineFactory::typeID() { return "Line"; }
GraphInfo LineFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new LineItem;
    GetValueHelper<LineItem> autoValue(item);

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
    /* 线段 */
    if (!GraphFactoryUtility::lineFromElement(item, element))
    {
        return {};
    }
    /* 箭头 */
    if (!GraphFactoryUtility::lineArrowFromElement(item, element))
    {
        return {};
    }

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(LineFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(LineFactory);
    info.setBorderColorData = BORDERCOLORDATAFUNC_TEMPLATE(LineFactory);
    return info;
}
bool LineFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const LineItem*>(info.item);
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
    /* 线段 */
    if (!GraphFactoryUtility::lineToElement(item, element))
    {
        return false;
    }
    /* 箭头 */
    GraphFactoryUtility::lineArrowToElement(item, element);
    return true;
}
void LineFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    auto item = dynamic_cast<LineItem*>(info.item);
    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    item->setArrowColor(color);
    item->setPenColor(color);
}
void LineFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    auto item = dynamic_cast<LineItem*>(info.item);
    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    item->setArrowColor(color);
    item->setPenColor(color);
}

void LineFactory::defaultSetBorderColorData(const GraphInfo& info, const QVariant&)
{
    auto item = dynamic_cast<LineItem*>(info.item);
    Q_ASSERT(item != nullptr);
}
