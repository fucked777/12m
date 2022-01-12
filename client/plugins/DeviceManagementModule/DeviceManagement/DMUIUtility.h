#ifndef DMUIUTILITY_H
#define DMUIUTILITY_H
#include "DynamicConfigToWidgetUtility.h"
#include "Utility.h"
#include <QGroupBox>
#include <QString>
#include <QVariant>
#include <tuple>

class QDomElement;
class QTreeWidget;
class QTabWidget;

struct FromInfo
{
    QString name;
    QList<CommonToWidgetInfo> fromList;
};
/* 树形结构 数据列表 */
struct TreeList
{
    CommonToWidgetInfo commonToWidgetInfo;
    QList<TreeList> child;
};

struct TreeInfo
{
    QString name;
    quint32 maxGroup{ 0 }; /* 最多有多少组参数 */
    TreeList treeList;
};

struct UIDataParameter
{
    QString type;
    QString name;
    FromInfo fromInfo;
    TreeInfo treeInfo;
};

struct DMFromWidgetInfo
{
    QGroupBox* widget{ nullptr };
    QList<DynamicWidgetInfo> fromWidgetInfo;
};
struct DMTreeWidgetInfo
{
    QTreeWidget* widget{ nullptr };
    QList<QMap<QString, DynamicWidgetInfo>> treeWidgetInfoList;
};

class DMUIUtility
{
public:
    static Optional<UIDataParameter> loadConfigXml(const QString& path);
    static OptionalNotValue parseXmlConfig(QDomElement& docElem, UIDataParameter& parameter);
    static OptionalNotValue parseXmlFrom(QDomElement& itemElem, QList<CommonToWidgetInfo>& parameterList);
    static OptionalNotValue parseXmlTree(QDomElement& itemElem, TreeList& child);
    static OptionalNotValue parseXmlAttribute(QDomElement& itemElem, TreeList& child);
    /* 创建普通的列表数据和树型数据 */
    static Optional<DMFromWidgetInfo> createFromWidget(const FromInfo& param);
    static Optional<DMTreeWidgetInfo> createTreeWidget(const TreeInfo&, int num);
    /* 树形控件有多组的,刷新多组 */
    static OptionalNotValue refreshTreeWidget(quint32 channelNum, QTabWidget* treeUI, DMTreeWidgetInfo&, const TreeInfo& treeInfo);
    /* 获取和设置树形控件的值 */
    static void setTreeWidgetData(QList<QMap<QString, DynamicWidgetInfo>>& treeListWidget, const QList<QVariantMap>& objDataMap);
    static void setTreeWidgetData(QTabWidget* treeUI, const TreeInfo& treeInfo, DMTreeWidgetInfo& attr, const QList<QVariantMap>& objDataMap);
    static Optional<QList<QVariantMap>> getTreeWidgetData(const QList<QMap<QString, DynamicWidgetInfo>>& treeListWidget);
    static QList<QVariantMap> getTreeWidgetDataNotCheck(const QList<QMap<QString, DynamicWidgetInfo>>& treeListWidget);
    static void setFromWidgetData(QList<DynamicWidgetInfo>& listWidget, const QVariantMap& objDataMap);
    static Optional<QVariantMap> getFromWidgetData(const QList<DynamicWidgetInfo>& listWidget);
};
#endif  // DMUIUTILITY_H
