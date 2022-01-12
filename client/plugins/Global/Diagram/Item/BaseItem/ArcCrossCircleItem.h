#ifndef ARCCROSSCIRCLEITEM_H
#define ARCCROSSCIRCLEITEM_H
#include "ItemBase.h"

class ArcCrossCircleItemImpl;
class ArcCrossCircleItem : public ItemBase
{
    Q_OBJECT
public:
    enum class DisplayType
    {
        Arc = 1,
        Cross = 2,
    };

    ArcCrossCircleItem(QGraphicsItem* parent = nullptr);
    ~ArcCrossCircleItem() override;

    /* 设置半径 */
    void setRadius(double r);
    double radius() const;
    void setDisplayType(DisplayType);
    DisplayType displayType() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void singleClick(const QString& id, bool);
    void doubleClick(const QString& id, bool);

private:
    ArcCrossCircleItemImpl* m_impl;
};

#endif  // ARCCROSSCIRCLEITEM_H
