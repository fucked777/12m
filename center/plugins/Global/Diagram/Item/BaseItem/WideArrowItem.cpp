#include "WideArrowItem.h"
#include "ItemUtility.h"
#include <QDebug>

class WideArrowItemImpl
{
    /* 水平看 */
    static constexpr double defaultArrowWidth = 12.0;
    static constexpr double defaultArrowHeight = 12.0;
    static constexpr double defaultTotalWidth = defaultArrowWidth * 3.0;
    static constexpr double defaultTotalHeight = defaultArrowWidth * 0.5;

public:
    WideArrowItem* self;
    QPolygonF vectorPoints;
    double arrowW;
    double arrowH;
    double totalW;
    double totalH;
    SourcePointPosition spoint;
    bool doubleArrow;
    QRectF boundingRect;
    QPainterPath path;

    WideArrowItemImpl(WideArrowItem* self_)
        : self(self_)
        , arrowW(defaultArrowWidth)
        , arrowH(defaultArrowHeight)
        , totalW(defaultTotalWidth)
        , totalH(defaultTotalHeight)
        , spoint(SourcePointPosition::Left_Middle)
        , doubleArrow(false)
    {
        calc();
    }
    void setDoubleArrow(bool doubleArrow_)
    {
        doubleArrow = doubleArrow_;
        calc();
    }
    void setSize(double totalW_, double totalH_, double arrowW_)
    {
        if (totalW_ < 2.0 * arrowW_)
        {
            return;
        }
        arrowW = arrowW_;
        arrowH = totalH_;
        totalW = totalW_;
        totalH = totalH_;
        calc();
    }
    void setSourcePointPosition(SourcePointPosition spoint_)
    {
        spoint = spoint_;
        calc();
    }
    void calc();
    void calcDouble();
    void centerCalcDouble();
    void leftCalcDouble();
    void rightCalcDouble();

    void calcSingle();
    void centerCalcSingle();
    void leftCalcSingle();
    void rightCalcSingle();
};
void WideArrowItemImpl::calc()
{
    vectorPoints.clear();
    if (doubleArrow)
    {
        calcDouble();
    }
    else
    {
        calcSingle();
    }
    QPainterPath path_;
    path_.setFillRule(Qt::FillRule::WindingFill);
    path_.addPolygon(vectorPoints);
    auto tempRectF = self->ItemBase::boundingRect();
    if (tempRectF.isValid())
    {
        path_.addRect(tempRectF);
    }
    path = path_;
    boundingRect = path_.boundingRect();
}
void WideArrowItemImpl::calcDouble()
{
    switch (spoint)
    {
    case SourcePointPosition::Left_Up:
    case SourcePointPosition::Left_Down:
    case SourcePointPosition::Left_Middle:
    {
        break;
    }
    case SourcePointPosition::Up_Middle:
    case SourcePointPosition::Down_Middle:
    case SourcePointPosition::Center:
    {
        centerCalcDouble();
        return;
    }
    case SourcePointPosition::Right_Up:
    case SourcePointPosition::Right_Middle:
    case SourcePointPosition::Right_Down:
    {
        rightCalcDouble();
        return;
    }
    }
    leftCalcDouble();
}
void WideArrowItemImpl::centerCalcDouble()
{
    auto tempBodyW = totalW - 2.0 * arrowW;
    auto tempBodyWDiv2 = tempBodyW * 0.5;
    // auto tempBodyH = totalH * 0.5;
    auto tempTotalHDiv2 = totalH * 0.5;
    auto tempTotalHDiv4 = totalH * 0.25;

    vectorPoints.append(QPointF(-(arrowW + tempBodyWDiv2), 0.0));
    vectorPoints.append(QPointF(-tempBodyWDiv2, -tempTotalHDiv2));
    vectorPoints.append(QPointF(-tempBodyWDiv2, -tempTotalHDiv4));
    vectorPoints.append(QPointF(tempBodyWDiv2, -tempTotalHDiv4));
    vectorPoints.append(QPointF(tempBodyWDiv2, -tempTotalHDiv2));
    vectorPoints.append(QPointF(arrowW + tempBodyWDiv2, 0));
    vectorPoints.append(QPointF(tempBodyWDiv2, tempTotalHDiv2));
    vectorPoints.append(QPointF(tempBodyWDiv2, tempTotalHDiv4));
    vectorPoints.append(QPointF(-tempBodyWDiv2, tempTotalHDiv4));
    vectorPoints.append(QPointF(-tempBodyWDiv2, tempTotalHDiv2));
}
void WideArrowItemImpl::leftCalcDouble()
{
    auto tempBodyW = totalW - 2.0 * arrowW;
    auto tempTotalHDiv2 = totalH * 0.5;
    auto tempTotalHDiv4 = totalH * 0.25;

    vectorPoints.append(QPointF(0.0, 0.0));
    vectorPoints.append(QPointF(arrowW, -tempTotalHDiv2));
    vectorPoints.append(QPointF(arrowW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(arrowW + tempBodyW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(arrowW + tempBodyW, -tempTotalHDiv2));
    vectorPoints.append(QPointF(totalW, 0));
    vectorPoints.append(QPointF(arrowW + tempBodyW, tempTotalHDiv2));
    vectorPoints.append(QPointF(arrowW + tempBodyW, tempTotalHDiv4));
    vectorPoints.append(QPointF(arrowW, tempTotalHDiv4));
    vectorPoints.append(QPointF(arrowW, tempTotalHDiv2));
}
void WideArrowItemImpl::rightCalcDouble()
{
    auto tempBodyW = totalW - 2.0 * arrowW;
    auto tempTotalHDiv2 = totalH * 0.5;
    auto tempTotalHDiv4 = totalH * 0.25;

    vectorPoints.append(QPointF(0.0, 0.0));
    vectorPoints.append(QPointF(-arrowW, -tempTotalHDiv2));
    vectorPoints.append(QPointF(-arrowW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(-(arrowW + tempBodyW), -tempTotalHDiv4));
    vectorPoints.append(QPointF(-(arrowW + tempBodyW), -tempTotalHDiv2));
    vectorPoints.append(QPointF(-totalW, 0));
    vectorPoints.append(QPointF(-(arrowW + tempBodyW), tempTotalHDiv2));
    vectorPoints.append(QPointF(-(arrowW + tempBodyW), tempTotalHDiv4));
    vectorPoints.append(QPointF(-arrowW, tempTotalHDiv4));
    vectorPoints.append(QPointF(-arrowW, tempTotalHDiv2));
}

void WideArrowItemImpl::calcSingle()
{
    vectorPoints.clear();

    switch (spoint)
    {
    case SourcePointPosition::Left_Up:
    case SourcePointPosition::Left_Down:
    case SourcePointPosition::Left_Middle:
    {
        break;
    }
    case SourcePointPosition::Up_Middle:
    case SourcePointPosition::Down_Middle:
    case SourcePointPosition::Center:
    {
        centerCalcSingle();
        return;
    }
    case SourcePointPosition::Right_Up:
    case SourcePointPosition::Right_Middle:
    case SourcePointPosition::Right_Down:
    {
        rightCalcSingle();
        return;
    }
    }
    leftCalcSingle();
}
void WideArrowItemImpl::centerCalcSingle()
{
    auto tempBodyW = totalW - arrowW;
    auto tempBodyWDiv2 = tempBodyW * 0.5;
    auto halfToalW = totalW * 0.5;
    auto tempTotalHDiv2 = totalH * 0.5;
    auto tempTotalHDiv4 = totalH * 0.25;

    vectorPoints.append(QPointF(-halfToalW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(tempBodyWDiv2, -tempTotalHDiv4));
    vectorPoints.append(QPointF(tempBodyWDiv2, -tempTotalHDiv2));
    vectorPoints.append(QPointF(halfToalW, 0));
    vectorPoints.append(QPointF(tempBodyWDiv2, tempTotalHDiv2));
    vectorPoints.append(QPointF(tempBodyWDiv2, tempTotalHDiv4));
    vectorPoints.append(QPointF(-halfToalW, tempTotalHDiv4));
}
void WideArrowItemImpl::leftCalcSingle()
{
    auto tempBodyW = totalW - arrowW;
    auto tempTotalHDiv2 = totalH * 0.5;
    auto tempTotalHDiv4 = totalH * 0.25;

    vectorPoints.append(QPointF(0.0, -tempTotalHDiv4));
    vectorPoints.append(QPointF(tempBodyW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(tempBodyW, -tempTotalHDiv2));
    vectorPoints.append(QPointF(totalW, 0));
    vectorPoints.append(QPointF(tempBodyW, tempTotalHDiv2));
    vectorPoints.append(QPointF(tempBodyW, tempTotalHDiv4));
    vectorPoints.append(QPointF(0.0, tempTotalHDiv4));
}
void WideArrowItemImpl::rightCalcSingle()
{
    // auto tempBodyW = totalW -  arrowW;
    auto tempTotalHDiv2 = totalH * 0.5;
    auto tempTotalHDiv4 = totalH * 0.25;

    vectorPoints.append(QPointF(0.0, 0.0));
    vectorPoints.append(QPointF(-arrowW, -tempTotalHDiv2));
    vectorPoints.append(QPointF(-arrowW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(-totalW, -tempTotalHDiv4));
    vectorPoints.append(QPointF(-totalW, tempTotalHDiv4));
    vectorPoints.append(QPointF(-arrowW, tempTotalHDiv4));
    vectorPoints.append(QPointF(-arrowW, tempTotalHDiv2));
}

// void WideArrowItemImpl::centerCalc()
//{
//    auto tempBodyW = totalW - 2.0 * arrowW;
//    auto tempBodyWDiv2 = tempBodyW * 0.5;
//    // auto tempBodyH = totalH * 0.5;
//    auto tempTotalHDiv2 = totalH * 0.5;
//    auto tempTotalHDiv4 = totalH * 0.25;

//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-(arrowW + tempBodyWDiv2), 0.0), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-tempBodyWDiv2, -tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-tempBodyWDiv2, -tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(tempBodyWDiv2, -tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(tempBodyWDiv2, -tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW + tempBodyWDiv2, 0), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(tempBodyWDiv2, tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(tempBodyWDiv2, tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-tempBodyWDiv2, tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-tempBodyWDiv2, tempTotalHDiv2), rotateAngle));
//}
// void WideArrowItemImpl::leftCalc()
//{
//    auto tempBodyW = totalW - 2.0 * arrowW;
//    auto tempTotalHDiv2 = totalH * 0.5;
//    auto tempTotalHDiv4 = totalH * 0.25;

//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(0.0, 0.0), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW, -tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW, -tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW + tempBodyW, -tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW + tempBodyW, -tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(totalW, 0), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW + tempBodyW, tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW + tempBodyW, tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW, tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(arrowW, tempTotalHDiv2), rotateAngle));
//}
// void WideArrowItemImpl::rightCalc()
//{
//    auto tempBodyW = totalW - 2.0 * arrowW;
//    auto tempTotalHDiv2 = totalH * 0.5;
//    auto tempTotalHDiv4 = totalH * 0.25;

//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(0.0, 0.0), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-arrowW, -tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-arrowW, -tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-(arrowW + tempBodyW), -tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-(arrowW + tempBodyW), -tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-totalW, 0), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-(arrowW + tempBodyW), tempTotalHDiv2), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-(arrowW + tempBodyW), tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-arrowW, tempTotalHDiv4), rotateAngle));
//    vectorPoints.append(ItemUtility::rotatePoint(QPointF(-arrowW, tempTotalHDiv2), rotateAngle));
//}

/****************************************************************/
/****************************************************************/
/****************************************************************/
/****************************************************************/
WideArrowItem::WideArrowItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new WideArrowItemImpl(this))
{
    setPenColor(Qt::black);
    setBrush(QBrush(Qt::NoBrush));
}
WideArrowItem::~WideArrowItem() { delete m_impl; }

void WideArrowItem::setSize(const QSize& total, double arrowW)
{
    m_impl->setSize(total.width(), total.height(), arrowW);
    update(boundingRect());
}
void WideArrowItem::setSize(double totalW, double totalH, double arrowW)
{
    m_impl->setSize(totalW, totalH, arrowW);
    update(boundingRect());
}
QSize WideArrowItem::size() const { return QSize(m_impl->totalW, m_impl->totalH); }
double WideArrowItem::arrawW() const { return m_impl->arrowW; }
void WideArrowItem::setDoubleArrow(bool doubleArrow_)
{
    m_impl->setDoubleArrow(doubleArrow_);
    update(boundingRect());
}
bool WideArrowItem::doubleArrow() const { return m_impl->doubleArrow; }
void WideArrowItem::setSourcePointPosition(SourcePointPosition spoint)
{
    m_impl->setSourcePointPosition(spoint);
    update(boundingRect());
}
SourcePointPosition WideArrowItem::sourcePointPosition() const { return m_impl->spoint; }
void WideArrowItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setBrush(brush());
    painter->setPen(pen());
    painter->setFont(font());

    painter->drawPolygon(m_impl->vectorPoints);
}
QPainterPath WideArrowItem::shape() const { return m_impl->path; }
QRectF WideArrowItem::boundingRect() const { return m_impl->boundingRect; }
void WideArrowItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), QVariant()); }
void WideArrowItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), QVariant()); }
