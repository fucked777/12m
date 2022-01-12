#include "PixmapFactory.h"
#include "GraphFactoryRegisterTemplate.h"
#include "GraphFactoryUtility.h"
#include "PixmapItem.h"
#include "Utility.h"
#include <QDomElement>
REGISTER_PRIVAE_CLASS_TEMPLATE(PixmapFactory, fromElement, toElement)
void PixmapFactory::changeItemDataFunc(SetItemDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_ITEMDATAFUNC_TEMPLATE(PixmapFactory, func);
    }
}
void PixmapFactory::changeGroupDataFunc(SetGroupDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_GROUPDATAFUNC_TEMPLATE(PixmapFactory, func);
    }
}

void PixmapFactory::changeBorderColorDataFunc(SetBorderColorDataFunc func)
{
    if (func != nullptr)
    {
        CHANGE_BORDERCOLORDATAFUNC_TEMPLATE(PixmapFactory, func);
    }
}

QString PixmapFactory::typeID() { return "Pixmap"; }
GraphInfo PixmapFactory::fromElement(const GraphGlobal& cGlobal, const QDomElement& element)
{
    GraphInfo info;
    auto item = new PixmapItem;
    GetValueHelper<PixmapItem> autoValue(item);
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
    /* 图片信息 */
    if (!GraphFactoryUtility::imageFromElement(item, cGlobal, element))
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
    info.setItemData = ITEMDATAFUNC_TEMPLATE(PixmapFactory);
    info.setGroupData = GROUPDATAFUNC_TEMPLATE(PixmapFactory);
    return info;
}
bool PixmapFactory::toElement(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto item = dynamic_cast<const PixmapItem*>(info.item);
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

    /* 图片信息 */
    if (!GraphFactoryUtility::imageToElement(item, cGlobal, element))
    {
        return {};
    }
    /* 大小 */
    if (item->isScaling())
    {
        GraphFactoryUtility::sizeToElement(item, element);
    }

    /* 解析原点位置 */
    GraphFactoryUtility::sourcePointPositionToElement(item, element);
    /* 旋转角度 */
    GraphFactoryUtility::rotationToElement(item, element);

    return true;
}

void PixmapFactory::defaultSetItemData(const GraphInfo& /*info*/, const QVariant&) {}
void PixmapFactory::defaultSetGroupData(const GraphInfo& /*info*/, const QVariant&) {}
void PixmapFactory::defaultSetBorderColorData(const GraphInfo& /*info*/, const QVariant&) {}
