#ifndef DOUBLEARROWITEM_H
#define DOUBLEARROWITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPainter>

/* 双箭头加一个圆 */
class DoubleArrowItemImpl;
class DoubleArrowItem : public ItemBase
{
    Q_OBJECT

public:
    DoubleArrowItem(QGraphicsItem* parent = nullptr);
    ~DoubleArrowItem() override;
    /* 设置半径 */
    void setRadius(double r);
    double radius() const;

    /* 设置箭头的颜色 */
    void setArrowColor(const QColor&);
    void setArrowColor(Qt::GlobalColor);

    QPainterPath shape() const override;
    QRectF boundingRect() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    DoubleArrowItemImpl* m_impl;
};

#endif  // DOUBLEARROWITEM_H
