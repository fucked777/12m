#include "TextFactory.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "ItemBase.h"
#include "TextItem.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(TextFactory, fromElement, toElement)
void TextFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(TextFactory, func);
    }
}
void TextFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(TextFactory, func);
    }
}

void TextFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(TextFactory, func);
    }
}

QString TextFactory::typeID() { return "Text"; }
GraphInfo TextFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new TextItem;
    GetValueHelper<TextItem> autoValue(item);

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

    info.item = autoValue.get();
    info.setItemData = ITEMDATAFUNC_TEMPLATE(TextFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(TextFactory);
    return info;
}
bool TextFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const TextItem*>(info.item);
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

    return true;
}
void TextFactory::defaultSetItemData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setFontColor(color);
    // info.item->setPenColor(color);
}
void TextFactory::defaultSetGroupData(const GraphInfo& info, const QVariant& data)
{
    Q_ASSERT(info.item != nullptr);
    //    auto item = dynamic_cast<RectItem*>(info.item);
    //    Q_ASSERT(item != nullptr);
    auto color = QColor::fromRgba(data.toUInt());
    info.item->setFontColor(color);
    // info.item->setPenColor(color);
}

void TextFactory::defaultSetBorderColorData(const GraphInfo& /*info*/, const QVariant&) {}
