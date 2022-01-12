#include "RectItem.h"
#include "ItemUtility.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

class RectItemImpl
{
    static constexpr double defaultWidth = 10;
    static constexpr double defaultHeight = 10;

public:
    RectItem* self;
    QRectF itemRectF;
    QRectF boundingRect;
    double w{ 0.0 };
    double h{ 0.0 };
    SourcePointPosition spp{ SourcePointPosition::Center };
    QPainterPath path;
    RectItemImpl(RectItem* self_)
        : self(self_)
    {
        setSize(defaultWidth, defaultHeight);
    }

    void setSize(double w_, double h_)
    {
        w = (w_ < defaultWidth) ? defaultWidth : w_;
        h = (h_ < defaultHeight) ? defaultHeight : h_;
        calc();
    }
    void setSourcePointPosition(SourcePointPosition position_)
    {
        spp = position_;
        calc();
    }

    void calc()
    {
        itemRectF = ItemUtility::calcRectFPos(spp, QPointF(0.0, 0.0), w, h);

        QPainterPath path_;
        path_.setFillRule(Qt::FillRule::WindingFill);
        path_.addRect(itemRectF);
        auto tempRectF = self->ItemBase::boundingRect();
        if (tempRectF.isValid())
        {
            path_.addRect(tempRectF);
        }
        path = path_;
        boundingRect = path_.boundingRect();
    }
};

RectItem::RectItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new RectItemImpl(this))
{
    setPenColor(Qt::black);
    setBrush(QBrush(Qt::NoBrush));
}
RectItem::~RectItem() { delete m_impl; }
void RectItem::setSize(double w, double h)
{
    m_impl->setSize(w, h);
    update(boundingRect());
}
QSize RectItem::size() const { return QSize(m_impl->w, m_impl->h); }

QRectF RectItem::boundingRect() const { return m_impl->boundingRect; }
QPainterPath RectItem::shape() const { return m_impl->path; }

void RectItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(brush());
    painter->setPen(pen());

    painter->drawRect(m_impl->itemRectF);
}
void RectItem::setSourcePointPosition(SourcePointPosition position)
{
    m_impl->setSourcePointPosition(position);
    update(boundingRect());
}
SourcePointPosition RectItem::sourcePointPosition() const { return m_impl->spp; }
void RectItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), QVariant()); }
void RectItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), QVariant()); }
