#ifndef TWOARCCIRCLEITEM_H
#define TWOARCCIRCLEITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPainter>

class TwoArcCircleItemImpl;
class TwoArcCircleItem : public ItemBase
{
    Q_OBJECT
public:
    TwoArcCircleItem(QGraphicsItem* parent = nullptr);
    ~TwoArcCircleItem() override;
    /* 这个东东比较特殊 角度为0是斜着的是左上右下的状态 */
    void setRadius(double);
    double radius() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    TwoArcCircleItemImpl* m_impl;
};

#endif  // TWOARCCIRCLEITEM_H
