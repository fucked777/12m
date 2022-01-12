#ifndef ITEMUTILITY_H
#define ITEMUTILITY_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPointF>
#include <QPolygonF>
#include <QString>
#include <functional>

class ItemUtility
{
public:
    static QString joinPath(const QString& v1, const QString& v2);
    static double directionToAngle(Direction direction);
    /* 计算三角形 */
    static void calcArrow(double rotateAngle, double w, double h, QPolygonF& point,
                          SourcePointPosition sourcePoint = SourcePointPosition::Left_Middle, const QPointF& offset = QPointF(0.0, 0.0));
    static void calcArrow(double w, double h, QPolygonF& point, SourcePointPosition sourcePoint = SourcePointPosition::Left_Middle,
                          const QPointF& offset = QPointF(0.0, 0.0));
    /* 计算当前宽和高情况下不同的中心位置的RectF */
    static QRectF calcRectFPos(SourcePointPosition pos, const QPointF& ptSrc, double w, double h);
    static void calcDivisibleValue(double);

    /*
     * ptSrc: 圆上某点(初始点);
     * ptRotationCenter: 圆心点为0.0;
     * angle: 旋转角度°  -- [angle * M_PI / 180]:将角度换算为弧度
     * [注意]angle 逆时针为正，顺时针为负
     */
    static QPointF rotatePoint(const QPointF& ptSrc, const QPointF& ptRotationCenter, double angle);
    static QPointF rotatePoint(const QPointF& ptSrc, double angle);
};

#endif  // ITEMUTILITY_H
