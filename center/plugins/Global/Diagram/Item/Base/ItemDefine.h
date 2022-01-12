#ifndef ITEMDEFINE_H
#define ITEMDEFINE_H
#include "ItemBase.h"
#include <QPointF>
#include <QPolygonF>
#include <QString>
#include <type_traits>
/*
 * 方向
 * 这方向就是特殊的角度而已
 * 统一以水平向右为0度,然后顺时针为正逆时针为负
 */
enum class Direction
{
    Up,
    Down,
    Left,
    Right,
    Left_Up,
    Left_Down,
    Right_Up,
    Right_Down,
};
struct ArrowInfo
{
    QPointF point;
    Direction direction{ Direction::Right };
    SourcePointPosition spp{ SourcePointPosition::Left_Middle };
    QSize size{ QSize(6.0, 6.0) };
};

#endif  // ITEMDEFINE_H
