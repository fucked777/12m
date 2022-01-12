#ifndef PIXMAPITEM_H
#define PIXMAPITEM_H
#include "ItemBase.h"
#include "ItemDefine.h"
#include <QPainter>

class PixmapItemImpl;
class PixmapItem : public ItemBase
{
    Q_OBJECT

public:
    PixmapItem(QGraphicsItem* parent = nullptr);
    ~PixmapItem() override;
    /* 设置图像路径 */
    void setImagePath(const QString& path);
    QString imagePath() const;
    /* 设置显示目标的大小 */
    void setSize(double w, double h);
    inline void setSize(const QSize& size) { setSize(size.width(), size.height()); }
    QSizeF size() const;
    bool isScaling() const;
    /* 设置原点位置 */
    void setSourcePointPosition(SourcePointPosition);
    SourcePointPosition sourcePointPosition() const;

    QPainterPath shape() const override;
    QRectF boundingRect() const override;

private:
    void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    PixmapItemImpl* m_impl;
};

#endif  // PIXMAPITEM_H
