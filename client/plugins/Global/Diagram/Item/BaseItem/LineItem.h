#ifndef LINEITEM_H
#define LINEITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"

class LineItemImpl;
class LineItem : public ItemBase
{
    Q_OBJECT
public:
    LineItem(QGraphicsItem* parent = nullptr);
    ~LineItem() override;

    /* 添加线 这是接在上一条线上的 */
    void addLine(double length, Direction direction);
    void addLine(const QPointF&); /* 此点是以当前item的0,0为基准的 */
    void addLine(double x, double y) { addLine(QPointF(x, y)); }
    void addLines(const QPolygonF&); /* 此点是以当前item的0,0为基准的 */
    QPolygonF lines() const;
    /* 添加三角形 */
    void addArrow(const ArrowInfo&);
    void addArrows(const QList<ArrowInfo>&);
    QList<ArrowInfo> arrows() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    /* 设置箭头的颜色 */
    void setArrowColor(const QColor&);
    void setArrowColor(Qt::GlobalColor);

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    LineItemImpl* m_impl;
};

#endif  // LINEITEM_H
