#include "TwoArcCircleItem.h"
#include <QDebug>

class TwoArcCircleItemImpl
{
    static constexpr double defaultRadius = 15;

public:
    TwoArcCircleItem* self;
    double radius;
    QRectF itemRectF;
    QRectF boundingRect;
    /* 开关内部两个通道的点 */
    QRectF recFChannel1;
    QRectF recFChannel2;
    int startAngle1;
    int spanAngle1;
    int startAngle2;
    int spanAngle2;
    QPainterPath path;

    TwoArcCircleItemImpl(TwoArcCircleItem* self_)
        : self(self_)
        , radius(defaultRadius)
    {
        calc();
    }
    void setRadius(double radius_)
    {
        radius = radius_;
        calc();
    }
    void calc()
    {
        /* 计算出绘制内部圆弧的坐标 */
        auto side = radius * 2.0;
        /* sqrt2/2  0.70710678118654752440084436210485 */
        // static constexpr double sqrt2Divide2 = 0.707106781186548;
        // static constexpr double sqrt2Divide2AddNegativeOne = 0.292893218813452;

        recFChannel1 = QRectF(-side, -side, side, side);
        recFChannel2 = QRectF(0, 0, side, side);
        startAngle1 = 0;
        spanAngle1 = -90 * 16;
        startAngle2 = 90 * 16;
        spanAngle2 = 90 * 16;
        itemRectF = QRectF(-radius, -radius, side, side);

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

        // auto sqrt2Divide2Radius = radius * sqrt2Divide2AddNegativeOne;
        //        switch (dir)
        //        {
        //        case Direction::Up:
        //        case Direction::Down:
        //        {
        //            recFChannel1 = QRectF(-radius, -side - sqrt2Divide2Radius, side, side);
        //            recFChannel2 = QRectF(-radius, sqrt2Divide2Radius, side, side);
        //            startAngle1 = -45 * 16;
        //            spanAngle1 = -90 * 16;
        //            startAngle2 = 45 * 16;
        //            spanAngle2 = 90 * 16;
        //            break;
        //        }
        //        case Direction::Left:
        //        case Direction::Right:
        //        {
        //            recFChannel1 = QRectF(-side - sqrt2Divide2Radius, -radius, side, side);
        //            recFChannel2 = QRectF(sqrt2Divide2Radius, -radius, side, side);
        //            startAngle1 = 45 * 16;
        //            spanAngle1 = -90 * 16;
        //            startAngle2 = 135 * 16;
        //            spanAngle2 = 90 * 16;
        //            break;
        //        }
        //        case Direction::Left_Up:
        //        case Direction::Right_Down:
        //        {
        //            recFChannel1 = QRectF(-side, -side, side, side);
        //            recFChannel2 = QRectF(0, 0, side, side);
        //            startAngle1 = 0;
        //            spanAngle1 = -90 * 16;
        //            startAngle2 = 90 * 16;
        //            spanAngle2 = 90 * 16;
        //            break;
        //        }
        //        case Direction::Left_Down:
        //        case Direction::Right_Up:
        //        {
        //            recFChannel1 = QRectF(-side, 0, side, side);
        //            recFChannel2 = QRectF(0, -side, side, side);
        //            startAngle1 = 0 * 16;
        //            spanAngle1 = 90 * 16;
        //            startAngle2 = 180 * 16;
        //            spanAngle2 = 90 * 16;

        //            break;
        //        }
        //        }
    }
};

TwoArcCircleItem::TwoArcCircleItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new TwoArcCircleItemImpl(this))
{
    setPenColor(Qt::black);
    setBrush(QBrush(Qt::NoBrush));
}
TwoArcCircleItem::~TwoArcCircleItem() { delete m_impl; }
QRectF TwoArcCircleItem::boundingRect() const { return m_impl->boundingRect; }
QPainterPath TwoArcCircleItem::shape() const { return m_impl->path; }
void TwoArcCircleItem::setRadius(double r)
{
    m_impl->setRadius(r);
    update(boundingRect());
}
double TwoArcCircleItem::radius() const { return m_impl->radius; }

void TwoArcCircleItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->setFont(font());

    painter->drawEllipse(m_impl->itemRectF);
    painter->drawArc(m_impl->recFChannel1, m_impl->startAngle1, m_impl->spanAngle1);
    painter->drawArc(m_impl->recFChannel2, m_impl->startAngle2, m_impl->spanAngle2);
}
void TwoArcCircleItem::mousePressEvent(QGraphicsSceneMouseEvent* event) { emit sg_singleClick(id(), rotation()); }
void TwoArcCircleItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) { emit sg_doubleClick(id(), rotation()); }
