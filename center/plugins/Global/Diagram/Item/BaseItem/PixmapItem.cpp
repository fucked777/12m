#include "PixmapItem.h"
#include "ItemUtility.h"
#include <QFile>
#include <QPixmap>
#include <QtMath>

#include <QDebug>
class PixmapItemImpl
{
public:
    PixmapItem* self;
    QString imagePath; /* 图像的路径 */
    QPixmap pixmap;    /* 绘制的图像 */
    QSizeF size;
    SourcePointPosition spp;
    QRectF itemRectF;   /* 绘制的区域 */
    QRectF displayRect; /* 显示的区域 */
    QRectF boundingRect;
    QPainterPath path;

    PixmapItemImpl(PixmapItem* self_)
        : self(self_)
        , spp(SourcePointPosition::Center)
    {
    }
    void setImagePath(const QString& path)
    {
        imagePath = path;
        calc();
    }
    void setSourcePointPosition(SourcePointPosition position_)
    {
        spp = position_;
        calc();
    }
    void setSize(double w_, double h_)
    {
        if (w_ > 0.0 && h_ > 0.0)
        {
            size = QSizeF(w_, h_);
        }
        calc();
    }
    void calc()
    {
        auto pixmap_ = QPixmap(imagePath);
        if (pixmap_.isNull())
        {
            return;
        }
        pixmap = pixmap_;
        displayRect = QRectF(pixmap.rect());
        if (size.isEmpty())
        {
            itemRectF = ItemUtility::calcRectFPos(spp, QPointF(0.0, 0.0), pixmap.width(), pixmap.height());
        }
        else
        {
            itemRectF = ItemUtility::calcRectFPos(spp, QPointF(0.0, 0.0), size.width(), size.height());
        }

        QPainterPath path_;
        path_.setFillRule(Qt::FillRule::WindingFill);
        path_.addRect(itemRectF);
        auto tempRectF = self->ItemBase::boundingRect();
        if (tempRectF.isValid())
        {
            path_.addRect(tempRectF);
        }
        boundingRect = path_.boundingRect();
        path = path_;
    }
};

PixmapItem::PixmapItem(QGraphicsItem* parent)
    : ItemBase(parent)
    , m_impl(new PixmapItemImpl(this))
{
}
PixmapItem::~PixmapItem() { delete m_impl; }
void PixmapItem::setSize(double w, double h)
{
    m_impl->setSize(w, h);
    update(boundingRect());
}
void PixmapItem::setImagePath(const QString& path)
{
    m_impl->setImagePath(path);
    update(boundingRect());
}
QString PixmapItem::imagePath() const { return m_impl->imagePath; }
void PixmapItem::setSourcePointPosition(SourcePointPosition spp)
{
    m_impl->setSourcePointPosition(spp);
    update(boundingRect());
}
SourcePointPosition PixmapItem::sourcePointPosition() const { return m_impl->spp; }
QSizeF PixmapItem::size() const { return m_impl->size; }
bool PixmapItem::isScaling() const { return m_impl->pixmap.size() == m_impl->size; }
QPainterPath PixmapItem::shape() const { return m_impl->path; }
QRectF PixmapItem::boundingRect() const { return m_impl->boundingRect; }

void PixmapItem::customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_impl->pixmap.isNull())
    {
        return;
    }

    painter->setBrush(brush());
    painter->setPen(pen());
    painter->drawPixmap(m_impl->itemRectF, m_impl->pixmap, m_impl->displayRect);
}
void PixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_singleClick(id(), QVariant()); }
void PixmapItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { emit sg_doubleClick(id(), QVariant()); }
