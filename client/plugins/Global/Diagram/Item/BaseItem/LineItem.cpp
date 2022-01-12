#include "LineItem.h"
#include "ItemUtility.h"
#include <QDebug>
#include <QtMath>

class LineItemImpl
{
public:
    static constexpr double defaultWidth = 6;
    static constexpr double defaultHeight = 6;
    LineItem* self;
    /* 构成此线段的所有顺序点 */
    QPolygonF pointF; /* 线段的构成点 */
    QList<ArrowInfo> arrowInfoList;
    QList<QPolygonF> arrowItem; /* 三角形 */
    QRectF boundingRect;
    QPainterPath path;

    /* 箭头的颜色 默认黑色 */
    QBrush arrowBrush;
    QPen arrowPen;

    LineItemImpl(LineItem* self_)
        : self(self_)
        , arrowBrush(Qt::black, Qt::SolidPattern)
        , arrowPen(Qt::NoPen)
    {
    }

    void calc()
    {
        arrowItem.clear();
        QPainterPath path_;
        path_.setFillRule(Qt::FillRule::WindingFill);
        QPolygonF tempPoint(3);
        path_.addPolygon(pointF);
        for (auto& info : arrowInfoList)
        {
            ItemUtility::calcArrow(ItemUtility::directionToAngle(info.direction), info.size.width(), info.size.height(), tempPoint, info.spp,
                                   info.point);
            arrowItem << tempPoint;
            path_.addPolygon(tempPoint);
        }

        auto tempRectF = self->ItemBase::boundingRect();
        if (tempRectF.isValid())
        {
            path_.addRect(tempRectF);
        }

        path = path_;
        boundingRect = path_.boundingRect();
    }
};

LineItem::LineItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new LineItemImpl(this))
{
    /* 默认 黑色的线条 */
    setPenColor(Qt::black);
    setBrushColor(Qt::black);
}
LineItem::~LineItem()
{
    delete m_impl;
    m_impl = nullptr;
}
QRectF LineItem::boundingRect() const { return m_impl->boundingRect; }
QPainterPath LineItem::shape() const { return m_impl->path; }
void LineItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->drawPolyline(m_impl->pointF);

    painter->setBrush(m_impl->arrowBrush);
    painter->setPen(m_impl->arrowPen);
    for (auto& item : m_impl->arrowItem)
    {
        painter->drawPolygon(item);
    }
}

void LineItem::addLine(const QPointF& newPoint)
{
    m_impl->pointF.append(newPoint);
    m_impl->calc();
    update(boundingRect());
}
void LineItem::addLines(const QPolygonF& newPoints)
{
    m_impl->pointF.append(newPoints);
    m_impl->calc();
    update(boundingRect());
}
QPolygonF LineItem::lines() const { return m_impl->pointF; }
void LineItem::addLine(double length, Direction direction)
{
    /* 1.4142135623730950488016887242097 */
    static constexpr double sqrt2Divide2 = 0.707106781186548;
    if (qFuzzyIsNull(length))
    {
        return;
    }
    auto newPoint = m_impl->pointF.last();
    switch (direction)
    {
    case Direction::Up:
    {
        newPoint.ry() -= length;
        break;
    }
    case Direction::Down:
    {
        newPoint.ry() += length;
        break;
    }
    case Direction::Left:
    {
        newPoint.rx() -= length;
        break;
    }
    case Direction::Right:
    {
        newPoint.rx() += length;
        break;
    }
    case Direction::Left_Up:
    {
        auto tempValue = sqrt2Divide2 * length;
        newPoint.rx() -= tempValue;
        newPoint.ry() -= tempValue;
        break;
    }
    case Direction::Left_Down:
    {
        auto tempValue = sqrt2Divide2 * length;
        newPoint.rx() -= tempValue;
        newPoint.ry() += tempValue;
        break;
    }
    case Direction::Right_Up:
    {
        auto tempValue = sqrt2Divide2 * length;
        newPoint.rx() += tempValue;
        newPoint.ry() -= tempValue;
        break;
    }
    case Direction::Right_Down:
    {
        auto tempValue = sqrt2Divide2 * length;
        newPoint.rx() += tempValue;
        newPoint.ry() += tempValue;
        break;
    }
    }

    m_impl->pointF.append(newPoint);
    m_impl->calc();
    update(boundingRect());
}
void LineItem::addArrow(const ArrowInfo& info)
{
    m_impl->arrowInfoList << info;
    m_impl->calc();
    update(boundingRect());
}
void LineItem::addArrows(const QList<ArrowInfo>& infoList)
{
    m_impl->arrowInfoList << infoList;
    m_impl->calc();
    update(boundingRect());
}
QList<ArrowInfo> LineItem::arrows() const { return m_impl->arrowInfoList; }

void LineItem::setArrowColor(const QColor& color)
{
    if (m_impl->arrowBrush.style() == Qt::NoBrush)
    {
        m_impl->arrowBrush.setStyle(Qt::SolidPattern);
    }
    m_impl->arrowBrush.setColor(color);
}
void LineItem::setArrowColor(Qt::GlobalColor color) { setArrowColor(QColor(color)); }
void LineItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), QVariant()); }
void LineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), QVariant()); }
