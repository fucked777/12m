#include "ArrowItem.h"
#include "ItemUtility.h"
#include <QtMath>

#include <QDebug>
class ArrowItemImpl
{
    static constexpr double defaultWidth = 6;
    static constexpr double defaultHeight = 6;

public:
    ArrowItem* self;
    QSizeF size;
    /* 箭头的3个点 */
    QPolygonF point;
    QRectF boundingRect;
    QPainterPath path;

    ArrowItemImpl(ArrowItem* self_)
        : self(self_)
        , size(defaultWidth, defaultHeight)
    {
        point.resize(3);
        calc();
    }
    void setSize(double w_, double h_)
    {
        size = QSizeF((w_ <= 0) ? defaultWidth : w_, (h_ <= 0) ? defaultHeight : h_);

        calc();
    }
    void calc()
    {
        ItemUtility::calcArrow(size.width(), size.height(), point, SourcePointPosition::Left_Middle);

        QPainterPath path_;
        path_.setFillRule(Qt::FillRule::WindingFill);
        path_.addPolygon(point);
        auto tempRectF = self->ItemBase::boundingRect();
        if (tempRectF.isValid())
        {
            path_.addRect(tempRectF);
        }
        boundingRect = path_.boundingRect();
        path = path_;
    }
};

ArrowItem::ArrowItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new ArrowItemImpl(this))
{
    /* 默认是黑色实心的箭头 */
    setPenColor(Qt::black);
    setBrushColor(Qt::black);
}
ArrowItem::~ArrowItem() { delete m_impl; }
void ArrowItem::setSize(double w, double h)
{
    m_impl->setSize(w, h);
    update(boundingRect());
}
QSizeF ArrowItem::size() const { return m_impl->size; }
QPainterPath ArrowItem::shape() const { return m_impl->path; }
QRectF ArrowItem::boundingRect() const { return m_impl->boundingRect; }

void ArrowItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->drawConvexPolygon(m_impl->point);
}
void ArrowItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), QVariant()); }
void ArrowItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), QVariant()); }
