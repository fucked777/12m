#include "ItemBase.h"
#include "ItemUtility.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QUuid>
#include <QtMath>

class ItemBaseImpl
{
public:
    QBrush brush;
    QPen pen;
    QPen textPen;
    QFont font;
    ItemFontInfo fontInfo;
    QString id;      /* 当前图元的唯一ID,为空会默认创建一个,但是可读性很差 */
    QString textRaw; /* 原始文本 */
    QString text;    /* 要绘制的文本 */
    QPointF ptSrc;
    QRectF itemRect; /* 文本绘制的矩形,此位置是以当前图元的0,0 点作为基准的 */
    QPainterPath path;
    SourcePointPosition textSPP; /* 文字是在矩形框中的,此位置是文字 怎样放在这个point上 */
    DrawTextMode drawTextMode;   /* 文字的放置模式 */
    bool wrap;                   /* 自动换行 */
    int drawTextFlag;

    ItemBaseImpl()
        : brush(Qt::NoBrush)
        , pen(Qt::black)
        , textPen(Qt::black)
        , font()
        , id(QUuid::createUuid().toString())
        , textSPP(SourcePointPosition::Center)
        , drawTextMode(DrawTextMode::Horizontal)
        , wrap(true)
        , drawTextFlag(Qt::AlignCenter | Qt::TextWordWrap)
    {
    }
    void setText(const QPointF& point_, const QString& text_, bool wrap_, SourcePointPosition pos_, DrawTextMode mode_)
    {
        ptSrc = point_;
        textRaw = text_;
        wrap = wrap_;
        textSPP = pos_;
        drawTextMode = mode_;
        calc();
    }

    void calc();
};

void ItemBaseImpl::calc()
{
    if (textRaw.isEmpty())
    {
        return;
    }
    drawTextFlag = (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextShowMnemonic);
    /* 处理文字 */
    switch (drawTextMode)
    {
    case DrawTextMode::Horizontal:
    {
        /* 水平放置文字,是自动换行只需要替换空格就行 */
        if (wrap)
        {
            text = textRaw.split(" ").join("\n");
        }
        /* 水平放置文字,如果不是自动换行那就什么都不做 */
        else
        {
            text = textRaw;
            drawTextFlag = (Qt::AlignCenter | Qt::TextShowMnemonic);
        }

        break;
    }
    case DrawTextMode::Vertical:
    {
        /* 垂直放置文字,直接把所有的文字中间加上一个\n就行 */
        text = textRaw.split("").join("\n");
        break;
    }
    }

    /* 文字处理完了,开始处理文字绘制矩形 */
    auto tempSize = font.pointSize() * text.length() * 3;
    QFontMetrics fontMetrics(font);

    QRect rect(0, 0, tempSize, tempSize);
    rect = fontMetrics.boundingRect(rect, drawTextFlag, text);
    /* 现在知道文字的坐标,文字占用的大小了 */
    itemRect = ItemUtility::calcRectFPos(textSPP, ptSrc, rect.width(), rect.height());
    QPainterPath path_;
    path_.addRect(itemRect);
    path = path_;
}
ItemBase::ItemBase(QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_baseImpl(new ItemBaseImpl)
{
}
ItemBase::~ItemBase()
{
    delete m_baseImpl;
    m_baseImpl = nullptr;
}

void ItemBase::setPen(const QPen& pen)
{
    m_baseImpl->pen = pen;
    update(boundingRect());
}
void ItemBase::setFontColor(const QColor& color) { m_baseImpl->textPen.setColor(color); }
void ItemBase::setFontColor(Qt::GlobalColor color) { setFontColor(QColor(color)); }
void ItemBase::setFontInfo(const ItemFontInfo& fontInfo)
{
    setFontColor(QColor::fromRgba(fontInfo.rgba));
    m_baseImpl->fontInfo = fontInfo;
    QFontDatabase fd;
    m_baseImpl->font = fd.font(fontInfo.family, fontInfo.fontStyles, fontInfo.size);
    m_baseImpl->font.setStrikeOut(fontInfo.strikeOut);
    m_baseImpl->font.setUnderline(fontInfo.underline);
    update(boundingRect());

    /* 计算和加载字体 */
    //    if (fontInfo.custom)
    //    {
    //        auto fontId = QFontDatabase::addApplicationFont(ItemUtility::joinPath(fontInfo.fontDir, fontInfo.name));
    //        auto families = QFontDatabase::applicationFontFamilies(fontId);

    //        if (families.isEmpty())
    //        {
    //            m_baseImpl->font = QFont();
    //        }
    //        else
    //        {
    //            QFontDatabase fd;
    //            m_baseImpl->font = fd.font(families.at(0), fontInfo.fontStyles, fontInfo.size);
    //        }

    //        m_baseImpl->font.setStrikeOut(fontInfo.strikeOut);
    //        m_baseImpl->font.setUnderline(fontInfo.underline);
    //    }
    //    else
    //    {
    //        /* 本地字体 */
    //        /* 非自定义字体 排序是 ->  自定义字体 familie fontStyles pointSize 删除线 下划线 字体颜色 */
    //        QFontDatabase fd;
    //        m_baseImpl->font = fd.font(fontInfo.name, fontInfo.fontStyles, fontInfo.size);
    //        m_baseImpl->font.setStrikeOut(fontInfo.strikeOut);
    //        m_baseImpl->font.setUnderline(fontInfo.underline);
    //    }
    //    update(boundingRect());
}
void ItemBase::setBrush(const QBrush& brush)
{
    m_baseImpl->brush = brush;
    update(boundingRect());
}
void ItemBase::setPenColor(const QColor& color)
{
    if (m_baseImpl->pen.style() == Qt::NoPen)
    {
        m_baseImpl->pen.setStyle(Qt::SolidLine);
    }
    m_baseImpl->pen.setColor(color);
    update(boundingRect());
}
void ItemBase::setPenColor(Qt::GlobalColor color)
{
    setPenColor(QColor(color));
    update(boundingRect());
}

void ItemBase::setBrushColor(const QColor& color)
{
    if (m_baseImpl->brush.style() == Qt::NoBrush)
    {
        m_baseImpl->brush.setStyle(Qt::SolidPattern);
    }
    m_baseImpl->brush.setColor(color);
    update(boundingRect());
}
void ItemBase::setBrushColor(Qt::GlobalColor color)
{
    setBrushColor(QColor(color));
    update(boundingRect());
}

QPen ItemBase::pen() const { return m_baseImpl->pen; }

QFont ItemBase::font() const { return m_baseImpl->font; }
ItemFontInfo ItemBase::fontInfo() const { return m_baseImpl->fontInfo; }
QBrush ItemBase::brush() const { return m_baseImpl->brush; }
void ItemBase::setID(const QString& id)
{
    if (id.isEmpty())
    {
        return;
    }
    m_baseImpl->id = id;
}
QString ItemBase::id() const { return m_baseImpl->id; }

void ItemBase::customPaint(QPainter* /*painter*/, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {}

void ItemBase::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    customPaint(painter, option, widget);
    /* 绘制文本为空,直接忽略 */
    if (m_baseImpl->text.isEmpty())
    {
        return;
    }
    painter->setPen(m_baseImpl->textPen);
    painter->setFont(font());
    painter->drawText(m_baseImpl->itemRect, m_baseImpl->drawTextFlag, m_baseImpl->text);
    // painter->drawRect(m_baseImpl->textRect);
}

void ItemBase::setText(const QPointF& point, const QString& text, DrawTextMode mode, SourcePointPosition pos, bool wrap)
{
    m_baseImpl->setText(point, text, wrap, pos, mode);
}
QString ItemBase::text() const { return m_baseImpl->textRaw; }
SourcePointPosition ItemBase::textSourcePointPosition() const { return m_baseImpl->textSPP; }
QPointF ItemBase::textStartPoint() const { return m_baseImpl->ptSrc; }
DrawTextMode ItemBase::drawTextMode() const { return m_baseImpl->drawTextMode; }
bool ItemBase::wrap() const { return m_baseImpl->wrap; }

QRectF ItemBase::boundingRect() const { return m_baseImpl->itemRect; }
QPainterPath ItemBase::shape() const { return m_baseImpl->path; }
void ItemBase::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mousePressEvent(event);
    emit sg_singleClick(id(), m_baseImpl->textRaw);
}

void ItemBase::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mouseDoubleClickEvent(event);
    emit sg_doubleClick(id(), m_baseImpl->textRaw);
}
