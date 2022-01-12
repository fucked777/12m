#ifndef AUTORUNPOLICYUTILITY_H
#define AUTORUNPOLICYUTILITY_H
#include "DynamicConfigToWidgetUtility.h"
#include <QGroupBox>
#include <QString>
#include <QVariant>
#include <tuple>
#include <type_traits>

class QDomElement;
class QTreeWidget;

/* 树形结构 数据列表 */
struct TreeList
{
    CommonToWidgetInfo commonToWidgetInfo;
    QList<TreeList> child;
};

struct TreeInfo
{
    QString name;
    TreeList treeList;
};
struct ATTreeWidgetInfo
{
    QTreeWidget* widget{ nullptr };
    QMap<QString, DynamicWidgetInfo> treeWidgetMap;
};

/*
 * 这里认为所有的key都是不一样的
 * 如果可能相同那生成的数据json也可能会嵌套很多层业务上根本不好获取
 * 所以认为所有的key都不一样,然后最终生成的json就是一个大Map
 * 业务上会方便很多,但是对于ui上还是会以正常的嵌套来解析
 */
class AutorunPolicyUtility
{
public:
    static Optional<QList<TreeInfo>> loadConfig(const QString& dir);
    static Optional<TreeInfo> loadItemConfigXml(const QString& path);
    static OptionalNotValue parseXmlConfig(QDomElement& docElem, TreeList& child);

    static Optional<ATTreeWidgetInfo> createTreeWidget(const TreeList&);
};
#endif  // AUTORUNPOLICYUTILITY_H
