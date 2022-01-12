#ifndef GRAPHFACTORY_H
#define GRAPHFACTORY_H
#include "ItemBase.h"
#include "Utility.h"
#include <QBrush>
#include <QDomElement>
#include <QFont>
#include <QMap>
#include <QPen>
#include <QString>
#include <functional>
#include <type_traits>

class ItemBase;
struct GraphInfo;
struct GraphGlobal;

/* 创建图元的函数 */
using GraphFromElenemtFunc = std::function<GraphInfo(const GraphGlobal&, const QDomElement&)>;
using GraphToElenemtFunc = std::function<bool(const GraphInfo&, const GraphGlobal&, QDomElement&)>;
/* 图元单独设置数据的函数 */
using SetItemDataFunc = std::function<void(const GraphInfo& info, const QVariant&)>;
/* 图元组参数设置的函数 */
using SetGroupDataFunc = std::function<void(const GraphInfo& info, const QVariant&)>;
/* 图元单独设置边框颜色的函数 */
using SetBorderColorDataFunc = std::function<void(const GraphInfo& info, const QVariant&)>;

struct GraphInfo
{
    QString typeID;                            /* 当前图元的类型ID,同类型的图元是相同的 */
    QString graphicsID;                        /* 每个图元单独的ID,此ID必须存在 */
    QStringList groupID;                       /* 组ID, */
    ItemBase* item{ nullptr };                 /* 当前的图元 */
    SetItemDataFunc setItemData;               /* 单独设置数据的函数 */
    SetGroupDataFunc setGroupData;             /* 组参数设置的函数 */
    SetBorderColorDataFunc setBorderColorData; /* 单独设置边框的函数 */
};

/* 一些图元的全局配置 */
struct GraphGlobal
{
    QBrush brush;
    QPen pen;
    ItemFontInfo fontInfo;
    QString curDir;    /* 当前的目录 此框图加载的资源文件都是在此目录下的此目录是固定的就是配置文件所在目录 */
    double zAxis{ 0 }; /* 默认的Z */
};
class QGraphicsScene;
struct GraphicsItemInfo
{
    /*
     * 警告:销毁时会全部以graphicsItemMap为基准销毁,对graphicsGroupMap的销毁操作是未定义的
     * 警告:销毁时会全部以graphicsItemMap为基准销毁,对graphicsGroupMap的销毁操作是未定义的
     * 警告:销毁时会全部以graphicsItemMap为基准销毁,对graphicsGroupMap的销毁操作是未定义的
     */
    /* 所有的图元映射的Map id是配置文件中的ID */
    QMap<QString, GraphInfo> graphicsItemMap;
    /* 这个是按图元的组ID分组 */
    QMap<QString, QList<GraphInfo>> graphicsGroupMap;
    /* 图元的全局信息,在保存的时候会用到 */
    GraphGlobal graphGlobal;
};

/*
 * 图元类型是区分大小写的
 *
 * 创建规则是如果有一点儿错误则忽略当前的图元
 */
class GraphFactory
{
public:
    /* 注册图元 */
    static void registerGraphType(const QString& typeID, GraphFromElenemtFunc from, GraphToElenemtFunc to);
    static GraphInfo createGraphItem(const QString& typeID, const GraphGlobal& cGlobal, const QDomElement& element);
    static bool saveGraphItem(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element);

    /*
     * 这里配置文件的解析是和图元耦合的
     * 省略了定义结构的部分
     * 因为图元的可能性太多了,定义结构会导致,结构数据会非常多
     * 且不可读
     * 这里直接解析配置文件
     * 先提取全局的数据
     * 然后根据图元的类型ID进行相应的图元创建操作
     */
    static Optional<GraphicsItemInfo> createBlockDiagram(const QString& path);
    static void destroyBlockDiagram(QGraphicsScene* scene, GraphicsItemInfo&);
};

#endif  // GRAPHFACTORY_H
