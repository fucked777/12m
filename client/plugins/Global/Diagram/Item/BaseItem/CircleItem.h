#ifndef CIRCLEITEM_H
#define CIRCLEITEM_H
#include "ItemBase.h"

/* 圆 */
class CircleItemImpl;
class CircleItem : public ItemBase
{
public:
    CircleItem(QGraphicsItem* parent = nullptr);
    ~CircleItem() override;
    /* 设置圆的半径 圆心是原点  */
    void setRadius(double radius);
    double radius() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    CircleItemImpl* m_impl;
};

#endif  // CCIRCLEITEM_H
