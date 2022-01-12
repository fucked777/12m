#ifndef ARROWITEM_H
#define ARROWITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPainter>

/* 箭头以水平向右为基准的,以底边中点为原点 */
class ArrowItemImpl;
class ArrowItem : public ItemBase
{
    Q_OBJECT

public:
    ArrowItem(QGraphicsItem* parent = nullptr);
    ~ArrowItem() override;
    /* 箭头大小 这里认为是箭头在一个矩形中 */
    void setSize(double w, double h);
    inline void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSizeF size() const;

    QPainterPath shape() const override;
    QRectF boundingRect() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    ArrowItemImpl* m_impl;
};

#endif  // ARROWITEM_H
