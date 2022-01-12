#include "CircleItem.h"
#include "ItemUtility.h"

class CircleItemImpl
{
public:
    static constexpr double defaultRadius = 1.0;
    CircleItem* self;
    double radius;
    QRectF itemRectF;
    QRectF boundingRect;
    QPainterPath path;
    CircleItemImpl(CircleItem* self_)
        : self(self_)
        , radius(defaultRadius)
    {
    }
    void setRadius(double radius_)
    {
        radius = radius_ < defaultRadius ? defaultRadius : radius_;
        calc();
    }

    void calc()
    {
        itemRectF = QRectF(-radius, -radius, radius + radius, radius + radius);
        auto tempRectF = self->ItemBase::boundingRect();

        QPainterPath path_;
        path_.setFillRule(Qt::FillRule::WindingFill);
        path_.addRect(itemRectF);
        if (tempRectF.isValid())
        {
            path_.addRect(tempRectF);
        }
        path = path_;
        boundingRect = path_.boundingRect();
    }
};

CircleItem::CircleItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new CircleItemImpl(this))
{
    setPenColor(Qt::black);
    setBrush(QBrush(Qt::NoBrush));
}
CircleItem::~CircleItem() { delete m_impl; }
void CircleItem::setRadius(double radius)
{
    m_impl->setRadius(radius);
    update(boundingRect());
}
double CircleItem::radius() const { return m_impl->radius; }
QRectF CircleItem::boundingRect() const { return m_impl->boundingRect; }
QPainterPath CircleItem::shape() const { return m_impl->path; }
void CircleItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setBrush(brush());
    painter->setPen(pen());

    painter->drawEllipse(m_impl->itemRectF);
}
void CircleItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), m_impl->radius); }
void CircleItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), m_impl->radius); }
