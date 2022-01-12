#include "GraphFactory.h"
#include "GraphFactoryUtility.h"
#include "ItemBase.h"
#include <QBrush>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QGraphicsScene>
#include <QMap>
#include <QMetaEnum>
#include <QPen>
#include <QStringList>

class GraphFactoryPrivate
{
public:
    struct GraphTypeObjInfo
    {
        GraphFromElenemtFunc from;
        GraphToElenemtFunc to;
    };
    static QMap<QString, GraphTypeObjInfo>& objMap()
    {
        static QMap<QString, GraphTypeObjInfo> g_ObjMap; /* 图元的创建与保存函数 */
        return g_ObjMap;
    }
    static QStringList& graphTypeList()
    {
        static QStringList g_GraphTypeList; /* 图元的类型列表,当前支持哪些图元 */
        return g_GraphTypeList;
    }
    static Optional<GraphicsItemInfo> parseConfig(const QString& curDir, QDomElement& element);
    static Optional<GraphicsItemInfo> parseGraphs(const GraphGlobal& graphGlobal, QDomElement& element);
};

void GraphFactory::registerGraphType(const QString& typeID, GraphFromElenemtFunc from, GraphToElenemtFunc to)
{
    Q_ASSERT(!GraphFactoryPrivate::objMap().contains(typeID));
    GraphFactoryPrivate::GraphTypeObjInfo info;
    info.from = from;
    info.to = to;
    GraphFactoryPrivate::objMap().insert(typeID, info);
    GraphFactoryPrivate::graphTypeList() << typeID;
}
GraphInfo GraphFactory::createGraphItem(const QString& typeID, const GraphGlobal& cGlobal, const QDomElement& element)
{
    auto find = GraphFactoryPrivate::objMap().find(typeID);
    if (find == GraphFactoryPrivate::objMap().end())
    {
        return {};
    }
    return find->from(cGlobal, element);
}
bool GraphFactory::saveGraphItem(const GraphInfo& info, const GraphGlobal& cGlobal, QDomElement& element)
{
    auto find = GraphFactoryPrivate::objMap().find(info.typeID);
    if (find == GraphFactoryPrivate::objMap().end())
    {
        return false;
    }
    return find->to(info, cGlobal, element);
}

/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/
/*************************************************************************************************/

Optional<GraphicsItemInfo> GraphFactoryPrivate::parseConfig(const QString& curDir, QDomElement& element)
{
    using ResType = Optional<GraphicsItemInfo>;
    GraphGlobal graphGlobal;
    graphGlobal.curDir = curDir;
    auto globalElement = element.firstChildElement("Global");
    GraphFactoryUtility::parseGlobal(graphGlobal, globalElement);

    auto graphElement = element.firstChildElement("Graph");
    if (graphElement.isNull())
    {
        return ResType(ErrorCode::InvalidData, "图元配置错误");
    }
    return parseGraphs(graphGlobal, graphElement);
}

Optional<GraphicsItemInfo> GraphFactoryPrivate::parseGraphs(const GraphGlobal& graphGlobal, QDomElement& element)
{
    using ResType = Optional<GraphicsItemInfo>;
    GraphicsItemInfo resultInfo;
    resultInfo.graphGlobal = graphGlobal;
    auto node = element.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto element = node.toElement();
        auto name = node.nodeName();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        auto item = GraphFactory::createGraphItem(name, graphGlobal, element);
        if (item.item == nullptr)
        {
            auto msg = QString("创建图元失败:%1").arg(name);
            qWarning() << msg;
            continue;
        }
        if (item.graphicsID.isEmpty())
        {
            delete item.item;
            continue;
        }
        if (resultInfo.graphicsItemMap.contains(item.graphicsID))
        {
            qWarning() << "ID已存在:" << item.graphicsID;
        }
        resultInfo.graphicsItemMap.insert(item.graphicsID, item);
        if (!item.groupID.isEmpty())
        {
            for (auto& id : item.groupID)
            {
                (resultInfo.graphicsGroupMap)[id] << item;
            }
        }
    }
    return ResType(resultInfo);
}
Optional<GraphicsItemInfo> GraphFactory::createBlockDiagram(const QString& path)
{
    using ResType = Optional<GraphicsItemInfo>;
    if (!QFile::exists(path))
    {
        return ResType(ErrorCode::NotFound, "文件未找到");
    }
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        auto msg = QString("图元配置打开错误:%1").arg(file.errorString());
        return ResType(ErrorCode::OpenFileError, msg);
    }
    /* 当前路径 */
    auto curDir = QFileInfo(path).absolutePath();

    QDomDocument doc;
    QString error;
    int errorRow = -1;
    int errorCol = -1;
    if (!doc.setContent(&file, &error, &errorRow, &errorCol))
    {
        auto msg = QString("图元配置解析错误%1,在%2第%3行第%4列").arg(error).arg(file.fileName()).arg(errorRow).arg(errorCol);
        return ResType(ErrorCode::XmlParseError, msg);
    }

    auto rootElement = doc.documentElement();
    return GraphFactoryPrivate::parseConfig(curDir, rootElement);
}
void GraphFactory::destroyBlockDiagram(QGraphicsScene* scene, GraphicsItemInfo& info)
{
    /* 这里有个问题,曾经我使用此方式去销毁,可能会导致崩溃
     * 现在我再试一次，如果出现崩溃则需要先
     * 循环
     * scene->removeItem()
     * 然后再delete
     */

    scene->clear();
    info.graphicsItemMap.clear();
    info.graphicsGroupMap.clear();
}
