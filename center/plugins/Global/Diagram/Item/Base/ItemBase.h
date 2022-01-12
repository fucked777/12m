#ifndef ITEMBASE_H
#define ITEMBASE_H
#include <QDebug>
#include <QGraphicsObject>
#include <QPainter>

/* 绘制文本的模式 */
enum class DrawTextMode
{
    Horizontal, /* 水平绘制 可通过空格换行 */
    Vertical,   /* 垂直绘制,此情况只能是一个文字一行 */
};

/* 原点的位置 */
enum class SourcePointPosition
{
    Up_Middle,    /* 上中 */
    Down_Middle,  /* 下中 */
    Left_Middle,  /* 左中 */
    Right_Middle, /* 右中 */
    Left_Up,      /* 左上 */
    Left_Down,    /* 左下 */
    Right_Up,     /* 右上 */
    Right_Down,   /* 右下 */
    Center,       /* 中心 */
};

struct ItemFontInfo
{
    bool custom{ false };    /* 自定义 */
    bool strikeOut{ false }; /* 删除线 */
    bool underline{ false }; /* 下划线 */
    QString name;            /* 为false是name和family 是一样的 为true name是名字 */
    QString family;          /* 为false是name和family 是一样的 为true name是名字  */
    QString fontStyles;
    int size{ 0 };
    QRgb rgba{ 0 }; /* 字体的RGBA */
};

//所有Item类的基类
class ItemBaseImpl;
class ItemBase : public QGraphicsObject
{
    Q_OBJECT
public:
    ItemBase(QGraphicsItem* parent = nullptr);
    ~ItemBase() override;

    /*
     * pen就是边框的颜色
     * Brush就是填充的颜色
     * 默认是黑色的线 然后没有填充
     */
    void setPen(const QPen& pen);
    void setFontInfo(const ItemFontInfo& fontInfo);
    void setFontColor(const QColor&);
    void setFontColor(Qt::GlobalColor);
    void setBrush(const QBrush& brush);
    void setPenColor(const QColor&);
    void setBrushColor(const QColor&);
    void setPenColor(Qt::GlobalColor);
    void setBrushColor(Qt::GlobalColor);

    QPen pen() const;
    QFont font() const;
    ItemFontInfo fontInfo() const;
    QBrush brush() const;

    void setID(const QString&);
    QString id() const;

    /* 设置绘制的文本
     * point 绘制的起始点是相对当前的(0,0)来讲的
     * text 绘制的文本
     * wrap 自动换行,当mode是Vertical时此值无效
     * pos 绘制的原点位置 这个位置是文字本身是一个矩形框然后这个矩形框的位置怎么放在point上
     * mode 水平放置还是垂直放置
     *
     * 注意这个DrawTextMode和SourcePointPosition只是针对文字本身的,与当前的图元无关
     */
    virtual void setText(const QPointF& point, const QString& text, DrawTextMode mode = DrawTextMode::Horizontal,
                         SourcePointPosition pos = SourcePointPosition::Center, bool wrap = true);
    QString text() const;
    SourcePointPosition textSourcePointPosition() const;
    QPointF textStartPoint() const;
    DrawTextMode drawTextMode() const;
    bool wrap() const;

    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;

    /* 获取当前的所有支持的图元 */
    static QStringList graphType();

protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    virtual void customPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
signals:
    void sg_singleClick(const QString& id, const QVariant& value);
    void sg_doubleClick(const QString& id, const QVariant& value);

private:
    ItemBaseImpl* m_baseImpl;
};

#endif  // ITEMBASE_H
