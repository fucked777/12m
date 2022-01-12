#include "ArcCrossCircleItem.h"
#include <QDebug>
#include <QPainter>

class ArcCrossCircleItemImpl
{
    static constexpr double defaultRadius = 15;

public:
    double radius;
    ArcCrossCircleItem::DisplayType displayType;
    QRectF curRectF;
    /* 开关内部两个通道的点 */
    QRectF recFChannel1;
    QRectF recFChannel2;
    int startAngle1;
    int spanAngle1;
    int startAngle2;
    int spanAngle2;

    ArcCrossCircleItemImpl()
        : radius(defaultRadius)
        , displayType(ArcCrossCircleItem::DisplayType::Arc)
    {
        calc();
    }
    void setRadius(double radius_)
    {
        radius = radius_;
        calc();
    }
    void setDisplayType(ArcCrossCircleItem::DisplayType displayType_)
    {
        displayType = displayType_;
        calc();
    }
    void calc()
    {
        /* 计算出绘制内部圆弧的坐标 */
        auto side = radius * 2.0;

        if (displayType == ArcCrossCircleItem::DisplayType::Arc)
        {
            recFChannel1 = QRectF(-side, -side, side, side);
            recFChannel2 = QRectF(0, 0, side, side);
            startAngle1 = 0;
            spanAngle1 = -90 * 16;
            startAngle2 = 90 * 16;
            spanAngle2 = 90 * 16;
        }

        curRectF = QRectF(-radius, -radius, side, side);
    }
};

ArcCrossCircleItem::ArcCrossCircleItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new ArcCrossCircleItemImpl)
{
    setPenColor(Qt::black);
    setBrush(QBrush(Qt::NoBrush));
}
ArcCrossCircleItem::~ArcCrossCircleItem() { delete m_impl; }

QRectF ArcCrossCircleItem::boundingRect() const { return m_impl->curRectF; }
void ArcCrossCircleItem::setRadius(double r)
{
    m_impl->setRadius(r);
    update(boundingRect());
}
double ArcCrossCircleItem::radius() const { return m_impl->radius; }
void ArcCrossCircleItem::setDisplayType(ArcCrossCircleItem::DisplayType var)
{
    if (displayType() == var)
    {
        return;
    }

    m_impl->setDisplayType(var);
    update(boundingRect());
}
ArcCrossCircleItem::DisplayType ArcCrossCircleItem::displayType() const { return m_impl->displayType; }
QPainterPath ArcCrossCircleItem::shape() const
{
    QPainterPath path;
    path.addEllipse(m_impl->curRectF);
    return path;
}
void ArcCrossCircleItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->setFont(font());

    painter->drawEllipse(m_impl->curRectF);
    if (m_impl->displayType == ArcCrossCircleItem::DisplayType::Arc)
    {
        painter->drawArc(m_impl->recFChannel1, m_impl->startAngle1, m_impl->spanAngle1);
        painter->drawArc(m_impl->recFChannel2, m_impl->startAngle2, m_impl->spanAngle2);
    }
    else
    {
        painter->drawLine(-m_impl->radius, 0, m_impl->radius, 0);
        painter->drawLine(0, -m_impl->radius, 0, m_impl->radius);
    }
}
void ArcCrossCircleItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), static_cast<int>(displayType())); }
void ArcCrossCircleItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), static_cast<int>(displayType())); }
