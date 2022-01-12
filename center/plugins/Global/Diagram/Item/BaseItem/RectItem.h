#ifndef RECTITEM_H
#define RECTITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPainter>

class RectItemImpl;
class RectItem : public ItemBase
{
    Q_OBJECT
public:
    RectItem(QGraphicsItem* parent = nullptr);
    ~RectItem() override;
    void setSize(double w, double h);
    inline void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSize size() const;
    /* 注意这个SourcePointPosition是图元本身的东东,与文字的SourcePointPosition无关 */
    void setSourcePointPosition(SourcePointPosition);
    SourcePointPosition sourcePointPosition() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    RectItemImpl* m_impl;
};

#endif  // RECTITEM_H
