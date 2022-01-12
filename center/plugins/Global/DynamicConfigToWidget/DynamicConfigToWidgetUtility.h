#ifndef DYNAMICCONFIGTOWIDGETUTILITY_H
#define DYNAMICCONFIGTOWIDGETUTILITY_H
#include "DynamicConfigToWidgetDefine.h"
#include "Utility.h"
/*
 * 这里认为所有的key都是不一样的
 * 如果可能相同那生成的数据json也可能会嵌套很多层业务上根本不好获取
 * 所以认为所有的key都不一样,然后最终生成的json就是一个大Map
 * 业务上会方便很多,但是对于ui上还是会以正常的嵌套来解析
 */
class QDomElement;
class DynamicConfigToWidgetUtility
{
public:
    static DynamicConfigToWidgetType controlTypeFromStr(const QString& raw);
    /*
     * 这里的控件都只是单个控件
     * 当类型是Unknown时 认为是非法配置
     */
    static Optional<DynamicWidgetInfo> createWidget(const CommonToWidgetInfo& toWidgetInfo);
    static Optional<QVariant> getWidgetValue(const DynamicWidgetInfo& info);
    static void setWidgetValue(const DynamicWidgetInfo& info, const QVariant& value);
    static void resetValue(const DynamicWidgetInfo& info);
    /* 提供一种解析单个数据的方法 */
    static Optional<CommonToWidgetInfo> parseXmlItem(QDomElement& element);
};
#endif  // DYNAMICCONFIGTOWIDGETUTILITY_H
