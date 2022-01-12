#include "DoubleArrowItem.h"
#include "ItemUtility.h"
#include <QDebug>
#include <QtMath>

class DoubleArrowItemImpl
{
    static constexpr double defaultRadius = 12;
    static constexpr double defaultArrSize = 6;
    static constexpr double defaultIncremental = 0.5;

public:
    DoubleArrowItem* self;
    double radius;
    /* 箭头的3个点 */
    QPolygonF point1;
    QPolygonF point2;
    /* 两条线 */
    QLineF line1;
    QLineF line2;

    QRectF itemRectF;
    QRectF boundingRect;

    /* 箭头的颜色 默认黑色 */
    QBrush arrowBrush;
    QPen arrowPen;
    QPainterPath path;

    DoubleArrowItemImpl(DoubleArrowItem* self_)
        : self(self_)
        , radius(defaultRadius)
        , arrowBrush(Qt::black, Qt::SolidPattern)
        , arrowPen(Qt::SolidLine)
    {
        point1.resize(3);
        point2.resize(3);
        calc();
    }
    void setRadius(double r)
    {
        radius = radius < defaultRadius ? defaultRadius : r;
        calc();
    }
    void calc()
    {
        /* sqrt3  1.7320508075688772935274463415059 */
        static constexpr double sqrt3Divide2 = 0.866025403784439;
        auto tempVertex = sqrt3Divide2 * radius - defaultIncremental;
        auto halfRadius = radius / 2 - defaultIncremental;

        ItemUtility::calcArrow(tempVertex, radius, point1, SourcePointPosition::Left_Down);
        ItemUtility::calcArrow(tempVertex, radius, point2, SourcePointPosition::Left_Up);

        line1 = QLineF(QPointF(0, -halfRadius), QPointF(-tempVertex, -halfRadius));
        line2 = QLineF(QPointF(0, halfRadius), QPointF(-tempVertex, halfRadius));

        itemRectF = QRectF(-radius, -radius, radius + radius, radius + radius);

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

DoubleArrowItem::DoubleArrowItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new DoubleArrowItemImpl(this))
{
    /*
     * 默认是黑色实心箭头+黑色的圆环
     */
}
DoubleArrowItem::~DoubleArrowItem() { delete m_impl; }

void DoubleArrowItem::setArrowColor(const QColor& color)
{
    m_impl->arrowPen.setColor(color);
    m_impl->arrowBrush.setColor(color);
}
void DoubleArrowItem::setArrowColor(Qt::GlobalColor color) { setArrowColor(QColor(color)); }

void DoubleArrowItem::setRadius(double r)
{
    m_impl->setRadius(r);
    update(boundingRect());
}
double DoubleArrowItem::radius() const { return m_impl->radius; }

QPainterPath DoubleArrowItem::shape() const { return m_impl->path; }
QRectF DoubleArrowItem::boundingRect() const { return m_impl->boundingRect; }
void DoubleArrowItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    /* 画圆 */
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->drawEllipse(m_impl->itemRectF);

    /* 画箭头 */
    painter->setBrush(m_impl->arrowBrush);
    painter->setPen(m_impl->arrowPen);
    painter->drawLine(m_impl->line1);
    painter->drawLine(m_impl->line2);
    painter->drawConvexPolygon(m_impl->point1);
    painter->drawConvexPolygon(m_impl->point2);
}
void DoubleArrowItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), m_impl->radius); }
void DoubleArrowItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), m_impl->radius); }
