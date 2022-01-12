#include "ItemDefaultDataSetChange.h"
#include "ArcCrossCircleFactory.h"
#include "ArcCrossCircleItem.h"
#include "TwoArcCircleFactory.h"
#include "TwoArcCircleItem.h"

class ItemDefaultDataSetChangePrivate
{
public:
    static void twoArcCircleSetItemData(const GraphInfo& info, const QVariant& value)
    {
        auto item = dynamic_cast<TwoArcCircleItem*>(info.item);
        Q_ASSERT(item != nullptr);
        item->setRotation(value.toDouble());
    }
    static void arcCrossCircleSetItemData(const GraphInfo& info, const QVariant& value)
    {
        auto item = dynamic_cast<ArcCrossCircleItem*>(info.item);
        Q_ASSERT(item != nullptr);
        item->setDisplayType(ArcCrossCircleItem::DisplayType(value.toInt()));
    }
};
void ItemDefaultDataSetChange::changeFunc()
{
    /* 先切换开关的默认数据设置事件 */
    TwoArcCircleFactory::changeItemDataFunc(&ItemDefaultDataSetChangePrivate::twoArcCircleSetItemData);
    ArcCrossCircleFactory::changeItemDataFunc(&ItemDefaultDataSetChangePrivate::arcCrossCircleSetItemData);
}
