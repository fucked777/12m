#ifndef GRAPHFACTORYUTILITY_H
#define GRAPHFACTORYUTILITY_H
#include "GraphFactory.h"
#include "ItemBase.h"
#include "ItemDefine.h"
#include "ItemUtility.h"
#include "Utility.h"
#include <QDir>
#include <QDomElement>
#include <QFile>
#include <QRegularExpression>

struct GraphGlobal;

/* 转换 */
QString& operator>>(QString& self, Optional<SourcePointPosition>& value);
SourcePointPosition& operator>>(SourcePointPosition& self, Optional<QString>& value);

QString& operator>>(QString& self, Optional<DrawTextMode>& value);
DrawTextMode& operator>>(DrawTextMode& self, Optional<QString>& value);

QString& operator>>(QString& self, Optional<Direction>& value);
Direction& operator>>(Direction& self, Optional<QString>& value);

/* 公共函数 */
struct GraphInfo;
class GraphFactoryUtility
{
public:
    /*
     * id的解析
     */
    static bool idFromElement(GraphInfo&, ItemBase* item, const QString& typeID, const QDomElement& element);
    static bool idToElement(const GraphInfo&, QDomElement& element);
    /* 文本部分的解析/保存
     * 解析失败认为错误
     * 保存错误则不保存
     */
    static bool textFromElement(ItemBase*, const QDomElement& element);
    static bool textToElement(const ItemBase*, const GraphGlobal& cGlobal, QDomElement& element);
    /* 公共部分的属性解析
     * 解析失败用公共的
     * 保存错误用公共的
     * 公共部分
     * brush="rgba style"
     * pen="rgba style width"
     * font="familie fontStyles pointSize 删除线 下划线 字体颜色"
     */
    static void brushFromElement(QBrush& brush, const QDomElement& element);
    static void brushToElement(const QBrush& brush, QDomElement& element);
    static void penFromElement(QPen& pen, const QDomElement& element);
    static void penToElement(const QPen& pen, QDomElement& element);
    static void fontFromElement(ItemFontInfo& fontInfo, const QDomElement& element);
    static void fontToElement(const ItemFontInfo& fontInfo, QDomElement& element);
    static void zFromElement(double& z, const QDomElement& element);
    static void zToElement(double z, QDomElement& element);

    static void parseGlobal(GraphGlobal& graphGlobal, QDomElement& element);
    static void restoresGlobal(const GraphGlobal& graphGlobal, QDomElement& element);
    static void globalFromElement(ItemBase*, const GraphGlobal& cGlobal, const QDomElement& element);
    static void globalToElement(const ItemBase*, const GraphGlobal& cGlobal, QDomElement& element);
    /* 方向解析
     * 解析失败用默认的
     */
    template<class T>
    static inline void rotationFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void rotationToElement(const T* item, QDomElement& element);

    /* 原点位置解析
     * 解析失败用默认的
     */
    template<class T>
    static inline void sourcePointPositionFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void sourcePointPositionToElement(const T* item, QDomElement& element);

    /* 大小/宽高...解析
     * 解析失败认为错误
     */
    template<class T>
    static inline bool sizeFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void sizeToElement(const T* item, QDomElement& element);

    /* 半径...解析
     * 解析失败认为错误
     */
    template<class T>
    static inline bool radiusFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void radiusToElement(const T* item, QDomElement& element);

    /* 解析图元位置
     * 解析失败认为错误
     */
    template<class T>
    static inline bool posFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void posToElement(const T* item, QDomElement& element);

    /* 解析线段
     * 解析失败认为错误
     */
    template<class T>
    static inline bool lineFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline bool lineToElement(const T* item, QDomElement& element);

    /* 解析箭头
     * 解析失败认为错误
     */
    template<class T>
    static inline bool lineArrowFromElement(T* item, const QDomElement& element);
    /* 解析失败则不转换 */
    template<class T>
    static inline void lineArrowToElement(const T* item, QDomElement& element);

    /* 解析图像
     * 解析失败认为错误
     */
    template<class T>
    static inline bool imageFromElement(T* item, const GraphGlobal& cGlobal, const QDomElement& element);
    /* 解析失败则不转换 */
    template<class T>
    static inline bool imageToElement(const T* item, const GraphGlobal& cGlobal, QDomElement& element);

    /* 大小/宽高...解析
     * 解析失败认为错误
     */
    template<class T>
    static inline bool wideArrowSizeFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void wideArrowSizeToElement(const T* item, QDomElement& element);

    template<class T>
    static inline void wideArrowDoubleFromElement(T* item, const QDomElement& element);
    template<class T>
    static inline void wideArrowDoubleToElement(const T* item, QDomElement& element);
};

template<class T>
void GraphFactoryUtility::rotationFromElement(T* item, const QDomElement& element)
{
    /* 方向的规则是 只有两个元素用空格隔开
     * 第一个是字符类型 A是角度 D是方向
     * dir="A 角度"/"D 方向"
     */
    auto direction = element.attribute("rotation").trimmed();
    if (direction.isEmpty())
    {
        return;
    }
    auto tempList = direction.split(" ", QString::SkipEmptyParts);
    if (tempList.size() != 2)
    {
        return;
    }
    if (tempList.at(0) == "A")
    {
        bool isOK = false;
        auto value = tempList[1].toDouble(&isOK);
        if (!isOK)
        {
            return;
        }
        item->setRotation(value);
        return;
    }
    if (tempList.at(0) == "D")
    {
        auto dir = Optional<Direction>::emptyOptional();
        tempList[1] >> dir;
        if (!dir.success())
        {
            return;
        }
        item->setRotation(ItemUtility::directionToAngle(dir.value()));
        return;
    }
}
template<class T>
void GraphFactoryUtility::rotationToElement(const T* item, QDomElement& element)
{
    /* 方向的规则是 只有两个元素用空格隔开
     * 第一个是字符类型 A是角度 D是方向
     * dir="A 角度"/"D 方向"
     */
    auto angle = item->rotation();
    if (qFuzzyIsNull(angle))
    {
        return;
    }
    auto msg = QString("A %1").arg(angle);
    element.setAttribute("rotation", msg);
}

template<class T>
void GraphFactoryUtility::sourcePointPositionFromElement(T* item, const QDomElement& element)
{
    /*
     * 原点位置只有字符串
     */
    auto sppStr = element.attribute("sourcePointPosition").trimmed();
    if (sppStr.isEmpty())
    {
        return;
    }
    auto spp = Optional<SourcePointPosition>::emptyOptional();
    sppStr >> spp;
    if (!spp.success())
    {
        return;
    }

    item->setSourcePointPosition(spp.value());
}
template<class T>
void GraphFactoryUtility::sourcePointPositionToElement(const T* item, QDomElement& element)
{
    auto sppStr = Optional<QString>::emptyOptional();
    auto spp = item->sourcePointPosition();
    spp >> sppStr;
    if (!sppStr.success())
    {
        return;
    }

    element.setAttribute("sourcePointPosition", sppStr.value());
}

template<class T>
bool GraphFactoryUtility::sizeFromElement(T* item, const QDomElement& element)
{
    /* 没有直接就使用默认的
     * 可以不设置但是不能写错
     * size="宽 高"
     */
    if (!element.hasAttribute("size"))
    {
        return true;
    }
    auto list = element.attribute("size").split(" ", QString::SkipEmptyParts);
    /* 只有一个就是宽和高相等 */
    if (list.size() != 2)
    {
        return false;
    }

    bool isOK1 = false;
    bool isOK2 = false;
    auto w = list.first().toDouble(&isOK1);
    auto h = list.last().toDouble(&isOK2);
    if (!(isOK1 & isOK2))
    {
        return false;
    }
    item->setSize(w, h);
    return true;
}
template<class T>
void GraphFactoryUtility::sizeToElement(const T* item, QDomElement& element)
{
    /* 没有直接就使用默认的
     * 规则是 一个数字是宽高相等,两个数字是宽和高分别设置
     * 可以不设置但是不能写错
     * size="宽 高"
     */
    auto size = item->size();
    auto msg = QString("%1 %2").arg(size.width()).arg(size.height());
    element.setAttribute("size", msg);
}
template<class T>
inline bool GraphFactoryUtility::wideArrowSizeFromElement(T* item, const QDomElement& element)
{
    /* 没有直接就使用默认的
     * 可以不设置但是不能写错
     * size="总宽 总高 箭头的宽"
     */
    if (!element.hasAttribute("size"))
    {
        return true;
    }
    auto list = element.attribute("size").split(" ", QString::SkipEmptyParts);
    /* 只有一个就是宽和高相等 */
    if (list.size() != 3)
    {
        return false;
    }

    bool isOK1 = false;
    bool isOK2 = false;
    bool isOK3 = false;
    auto totalW = list[0].toDouble(&isOK1);
    auto totalH = list[1].toDouble(&isOK2);
    auto arrawW = list[2].toDouble(&isOK3);
    if (!(isOK1 & isOK2 & isOK3))
    {
        return false;
    }
    item->setSize(totalW, totalH, arrawW);
    return true;
}
template<class T>
inline void GraphFactoryUtility::wideArrowSizeToElement(const T* item, QDomElement& element)
{
    /* 没有直接就使用默认的
     * 可以不设置但是不能写错
     * size="总宽 总高 箭头的宽"
     */
    auto size = item->size();
    auto msg = QString("%1 %2").arg(size.width()).arg(size.height()).arg(item->arrawW());
    element.setAttribute("size", msg);
}
template<class T>
inline void GraphFactoryUtility::wideArrowDoubleFromElement(T* item, const QDomElement& element)
{
    /* 没有直接就使用默认的
     * 可以不设置但是不能写错
     * doubleArrow="true/false"
     */
    auto doubleArrow = QVariant(element.attribute("doubleArrow").trimmed()).toBool();
    item->setDoubleArrow(doubleArrow);
}
template<class T>
inline void GraphFactoryUtility::wideArrowDoubleToElement(const T* item, QDomElement& element)
{
    /* 没有直接就使用默认的
     * 可以不设置但是不能写错
     * doubleArrow="true/false"
     */
    element.setAttribute("doubleArrow", item->doubleArrow() ? "true" : "false");
}

template<class T>
inline bool GraphFactoryUtility::radiusFromElement(T* item, const QDomElement& element)
{
    /*
     * radius="半径"
     */
    if (!element.hasAttribute("radius"))
    {
        return false;
    }
    bool isOK1 = false;
    auto tempR = element.attribute("radius").trimmed().toDouble(&isOK1);
    if (!isOK1)
    {
        return false;
    }
    item->setRadius(tempR);
    return true;
}
template<class T>
inline void GraphFactoryUtility::radiusToElement(const T* item, QDomElement& element)
{
    /*
     * radius="半径"
     */
    element.setAttribute("size", QString::number(item->radius()));
}

template<class T>
bool GraphFactoryUtility::posFromElement(T* item, const QDomElement& element)
{
    /* 没有直接就使用默认的
     * 可以不设置但是不能写错
     * point="x y"
     */
    if (!element.hasAttribute("pos"))
    {
        return true;
    }
    auto list = element.attribute("pos").split(" ", QString::SkipEmptyParts);
    if (list.size() == 2)
    {
        bool isOK1 = false;
        bool isOK2 = false;
        auto x = list.first().toDouble(&isOK1);
        auto y = list.last().toDouble(&isOK2);
        if (!(isOK1 & isOK2))
        {
            return false;
        }
        item->setPos(x, y);
        return true;
    }

    return false;
}
template<class T>
void GraphFactoryUtility::posToElement(const T* item, QDomElement& element)
{
    auto point = item->pos();
    auto msg = QString("%1 %2").arg(point.x()).arg(point.y());
    element.setAttribute("size", msg);
}
template<class T>
inline bool GraphFactoryUtility::lineFromElement(T* item, const QDomElement& element)
{
    /*
     * line="x,y x,y x,y x,y x,y x,y x,y ...."
     */
    auto tempList = element.attribute("line").trimmed().split(" ", QString::SkipEmptyParts);
    if (tempList.isEmpty())
    {
        return false;
    }

    QPolygonF points;
    bool isOK1 = false;
    bool isOK2 = false;
    for (auto& item : tempList)
    {
        auto tempPoint = item.split(",", QString::SkipEmptyParts);
        if (tempPoint.size() != 2)
        {
            return false;
        }
        auto x = tempPoint.first().toDouble(&isOK1);
        auto y = tempPoint.last().toDouble(&isOK2);
        if (!(isOK1 & isOK2))
        {
            return false;
        }
        points.append(QPointF(x, y));
    }
    /* 划线小于2个画不出线条 */
    if (points.size() < 2)
    {
        return false;
    }
    item->addLines(points);
    return true;
}
template<class T>
inline bool GraphFactoryUtility::lineToElement(const T* item, QDomElement& element)
{
    auto points = item->lines();
    if (points.size() < 2)
    {
        return false;
    }
    QString msg;
    for (auto& item : points)
    {
        msg.append(QString("%1 %2 ").arg(item.x()).arg(item.y()));
    }
    msg = msg.trimmed();
    element.setAttribute("line", msg);
    return true;
}
template<class T>
inline bool GraphFactoryUtility::lineArrowFromElement(T* item, const QDomElement& element)
{
    /*
     * arrow="x,y,w,h,dir,sourcePointPosition x,y,w,h,dir,sourcePointPosition x,y,w,h,dir,sourcePointPosition"
     */

    auto tempList = element.attribute("arrow").split(" ", QString::SkipEmptyParts);
    if (tempList.isEmpty())
    {
        return true;
    }
    QList<ArrowInfo> arrowList;
    ArrowInfo info;
    bool isOk1 = false;
    bool isOk2 = false;
    bool isOk3 = false;
    bool isOk4 = false;
    for (auto& item : tempList)
    {
        auto infoStrList = item.split(",", QString::SkipEmptyParts);
        if (infoStrList.size() != 6)
        {
            return false;
        }
        auto x = infoStrList[0].toDouble(&isOk1);
        auto y = infoStrList[1].toDouble(&isOk2);
        auto w = infoStrList[2].toDouble(&isOk3);
        auto h = infoStrList[3].toDouble(&isOk4);
        if (!(isOk1 & isOk2 & isOk3 & isOk4))
        {
            return false;
        }
        auto dir = Optional<Direction>::emptyOptional();
        infoStrList[4] >> dir;
        if (!dir.success())
        {
            return false;
        }
        auto spp = Optional<SourcePointPosition>::emptyOptional();
        infoStrList[5] >> spp;
        if (!spp.success())
        {
            return false;
        }
        info.direction = dir.value();
        info.point = QPointF(x, y);
        info.size = QSize(w, h);
        info.spp = spp.value();
        arrowList.append(info);
    }
    item->addArrows(arrowList);
    return true;
}
template<class T>
inline void GraphFactoryUtility::lineArrowToElement(const T* item, QDomElement& element)
{
    /*
     * arrow="x,y,w,h,dir,sourcePointPosition x,y,w,h,dir,sourcePointPosition x,y,w,h,dir,sourcePointPosition"
     */
    auto arrows = item->arrows();
    if (arrows.isEmpty())
    {
        return;
    }
    QString msg;
    for (auto& info : arrows)
    {
        auto dirStr = Optional<QString>::emptyOptional();
        info.direction >> dirStr;
        if (!dirStr.success())
        {
            return;
        }

        auto sppStr = Optional<QString>::emptyOptional();
        info.spp >> sppStr;
        if (!sppStr.success())
        {
            return;
        }

        auto tempMsg = QString("%1 %2 %3 %4 %5 %6")
                           .arg(info.point.x())
                           .arg(info.point.y())
                           .arg(info.size.width())
                           .arg(info.size.height())
                           .arg(dirStr.value(), sppStr.value());
        msg.append(tempMsg);
    }
    msg = msg.trimmed();
    element.setAttribute("arrow", msg);
}
template<class T>
inline bool GraphFactoryUtility::imageFromElement(T* item, const GraphGlobal& cGlobal, const QDomElement& element)
{
    /* image="图片名" */
    auto imageName = element.attribute("image").trimmed();
    /* 拼接路径 */
    auto path = ItemUtility::joinPath(cGlobal.curDir, imageName);
    if (!QFile::exists(path))
    {
        return false;
    }
    item->setImagePath(path);
    return true;
}
template<class T>
inline bool GraphFactoryUtility::imageToElement(const T* item, const GraphGlobal& /*cGlobal*/, QDomElement& element)
{
    /* image="图片名" */
    QString path = item->imagePath();
    if (!QFile::exists(path))
    {
        return false;
    }
    element.setAttribute("image", QFileInfo(path).fileName());
    return true;
}
#endif  // GRAPHFACTORYUTILITY_H
