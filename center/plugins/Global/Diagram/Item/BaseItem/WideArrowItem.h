#ifndef WIDEARROWITEM_H
#define WIDEARROWITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPainter>

class WideArrowItemImpl;
class WideArrowItem : public ItemBase
{
    Q_OBJECT

public:
    /* 全部的计算是认为箭头是水平的来计算的
     * 则默认情况下双箭头原点在左侧顶点
     * 水平为宽 竖直为高
     *
     * Up_Middle        原点在整个箭头的中心点
     * Down_Middle      原点在整个箭头的中心点
     * Center           原点在整个箭头的中心点
     * Left_Middle      原点在左侧的顶点
     * Left_Up          原点在左侧的顶点
     * Left_Down        原点在左侧的顶点
     * Right_Middle     原点在右侧的顶点
     * Right_Up         原点在右侧的顶点
     * Right_Down       原点在右侧的顶点
     *
     */
    WideArrowItem(QGraphicsItem* parent = nullptr);
    ~WideArrowItem() override;
    QPainterPath shape() const override;
    QRectF boundingRect() const override;

    /*
     * 水平来计算的
     * 水平为宽 竖直为高
     * total 总长总宽
     * arrowH 箭头高度
     * bodyW  身体宽度
     */
    void setSize(const QSize& total, double arrowW);
    void setSize(double totalW, double totalH, double arrowW);
    QSize size() const;
    double arrawW() const;
    /* 设置是否是双箭头,默认是单箭头 */
    void setDoubleArrow(bool);
    bool doubleArrow() const;
    /*
     * 原点位置
     * 默认双箭头 在其中一个箭头的顶点
     * 单箭头在无箭头端的中点
     */
    void setSourcePointPosition(SourcePointPosition);
    SourcePointPosition sourcePointPosition() const;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    WideArrowItemImpl* m_impl;
};

#endif  // WideArrowItem_H
