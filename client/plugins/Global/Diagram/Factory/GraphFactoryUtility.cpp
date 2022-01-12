#include "GraphFactoryUtility.h"
#include "GraphFactory.h"
#include <QFontDatabase>
#include <QMetaEnum>
static constexpr char gGlobalXMLAttrBrush[] = "brush";
static constexpr char gGlobalXMLAttrPen[] = "pen";
static constexpr char gGlobalXMLAttrFont[] = "font";
static constexpr char gGlobalXMLAttrZ[] = "z";
inline bool operator==(const ItemFontInfo& v1, const ItemFontInfo& v2)
{
    return (v1.custom == v2.custom) && (v1.strikeOut == v2.strikeOut) && (v1.underline == v2.underline) && (v1.name == v2.name) &&
           (v1.fontStyles == v2.fontStyles) && (v1.size == v2.size) && (v1.rgba == v2.rgba);
}
inline bool operator!=(const ItemFontInfo& v1, const ItemFontInfo& v2) { return !(v1 == v2); }
QString& operator>>(QString& self, Optional<SourcePointPosition>& value)
{
    using ResType = Optional<SourcePointPosition>;
    auto from = self.toLower();

    if (from == "upmiddle")
    {
        value = ResType(SourcePointPosition::Up_Middle);
        return self;
    }
    if (from == "downmiddle")
    {
        value = ResType(SourcePointPosition::Down_Middle);
        return self;
    }
    if (from == "leftmiddle")
    {
        value = ResType(SourcePointPosition::Left_Middle);
        return self;
    }
    if (from == "rightmiddle")
    {
        value = ResType(SourcePointPosition::Right_Middle);
        return self;
    }
    if (from == "leftup")
    {
        value = ResType(SourcePointPosition::Left_Up);
        return self;
    }
    if (from == "leftdown")
    {
        value = ResType(SourcePointPosition::Left_Down);
        return self;
    }
    if (from == "rightup")
    {
        value = ResType(SourcePointPosition::Right_Up);
        return self;
    }
    if (from == "rightdown")
    {
        value = ResType(SourcePointPosition::Right_Down);
        return self;
    }
    if (from == "center")
    {
        value = ResType(SourcePointPosition::Center);
        return self;
    }

    value = ResType(ErrorCode::NotFound, "未找到当前字符串对应的枚举");
    return self;
}
/* 安装字体 */
static void gfInstallFont(const QString& curDir, ItemFontInfo& fontInfo)
{
    /* 安装字体 */
    if (fontInfo.custom)
    {
        auto fontId = QFontDatabase::addApplicationFont(ItemUtility::joinPath(curDir, fontInfo.name));
        auto families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty())
        {
            fontInfo.family = families.at(0);
        }
    }
    else
    {
        fontInfo.family = fontInfo.name;
    }
}
SourcePointPosition& operator>>(SourcePointPosition& self, Optional<QString>& value)
{
    using ResType = Optional<QString>;
    switch (self)
    {
    case SourcePointPosition::Up_Middle:
    {
        value = ResType("UpMiddle");
        return self;
    }
    case SourcePointPosition::Down_Middle:
    {
        value = ResType("DownMiddle");
        return self;
    }
    case SourcePointPosition::Left_Middle:
    {
        value = ResType("LeftMiddle");
        return self;
    }
    case SourcePointPosition::Right_Middle:
    {
        value = ResType("RightMiddle");
        return self;
    }
    case SourcePointPosition::Left_Up:
    {
        value = ResType("LeftUp");
        return self;
    }
    case SourcePointPosition::Left_Down:
    {
        value = ResType("LeftDown");
        return self;
    }
    case SourcePointPosition::Right_Up:
    {
        value = ResType("RightUp");
        return self;
    }
    case SourcePointPosition::Right_Down:
    {
        value = ResType("RightDown");
        return self;
    }
    case SourcePointPosition::Center:
    {
        value = ResType("Center");
        return self;
    }
    }
    value = ResType(ErrorCode::NotFound, "未能将此枚举转换为字符串");
    return self;
}

QString& operator>>(QString& self, Optional<DrawTextMode>& value)
{
    using ResType = Optional<DrawTextMode>;
    auto from = self.toLower();

    if (from == "h")
    {
        value = ResType(DrawTextMode::Horizontal);
        return self;
    }
    else if (from == "v")
    {
        value = ResType(DrawTextMode::Vertical);
        return self;
    }
    value = ResType(ErrorCode::NotFound, "未找到当前字符串对应的枚举");
    return self;
}
DrawTextMode& operator>>(DrawTextMode& self, Optional<QString>& value)
{
    using ResType = Optional<QString>;
    switch (self)
    {
    case DrawTextMode::Horizontal:
    {
        value = ResType("H");
        return self;
    }
    case DrawTextMode::Vertical:
    {
        value = ResType("V");
        return self;
    }
    }
    value = ResType(ErrorCode::NotFound, "未能将此枚举转换为字符串");
    return self;
}

QString& operator>>(QString& self, Optional<Direction>& value)
{
    using ResType = Optional<Direction>;
    auto from = self.toLower();

    if (from == "up")
    {
        value = ResType(Direction::Up);
        return self;
    }
    if (from == "down")
    {
        value = ResType(Direction::Down);
        return self;
    }
    if (from == "left")
    {
        value = ResType(Direction::Left);
        return self;
    }
    if (from == "right")
    {
        value = ResType(Direction::Right);
        return self;
    }
    if (from == "leftup")
    {
        value = ResType(Direction::Left_Up);
        return self;
    }
    if (from == "leftdown")
    {
        value = ResType(Direction::Left_Down);
        return self;
    }
    if (from == "rightup")
    {
        value = ResType(Direction::Right_Up);
        return self;
    }
    if (from == "rightdown")
    {
        value = ResType(Direction::Right_Down);
        return self;
    }
    value = ResType(ErrorCode::NotFound, "未找到当前字符串对应的枚举");
    return self;
}
Direction& operator>>(Direction& self, Optional<QString>& value)
{
    using ResType = Optional<QString>;
    switch (self)
    {
    case Direction::Up:
    {
        value = ResType("Up");
        return self;
    }
    case Direction::Down:
    {
        value = ResType("Down");
        return self;
    }
    case Direction::Left:
    {
        value = ResType("Left");
        return self;
    }
    case Direction::Right:
    {
        value = ResType("Right");
        return self;
    }
    case Direction::Left_Up:
    {
        value = ResType("LeftUp");
        return self;
    }
    case Direction::Left_Down:
    {
        value = ResType("LeftDown");
        return self;
    }
    case Direction::Right_Up:
    {
        value = ResType("RightUp");
        return self;
    }
    case Direction::Right_Down:
    {
        value = ResType("RightDown");
        return self;
    }
    }
    value = ResType(ErrorCode::NotFound, "未能将此枚举转换为字符串");
    return self;
}

/********************************************************************/
bool GraphFactoryUtility::idFromElement(GraphInfo& info, ItemBase* item, const QString& typeID, const QDomElement& element)
{
    info.typeID = typeID;  // element.tagName();
    info.graphicsID = element.attribute("graphicsID").trimmed();
    if (info.graphicsID.isEmpty())
    {
        return false;
    }
    item->setID(info.graphicsID);
    info.groupID = element.attribute("groupID").trimmed().split(" ");
    return true;
}
bool GraphFactoryUtility::idToElement(const GraphInfo& info, QDomElement& element)
{
    if (info.graphicsID.isEmpty())
    {
        return false;
    }
    element.setTagName(info.typeID);
    element.setAttribute("graphicsID", info.graphicsID);
    element.setAttribute("groupID", info.groupID.join(" "));
    return true;
}
bool GraphFactoryUtility::textFromElement(ItemBase* item, const QDomElement& element)
{
    auto text = element.attribute("text");
    if (text.isEmpty())
    {
        return true;
    }
    /* 文字的配置布局是 x y model spos warp */
    if (!element.hasAttribute("textAttr"))
    {
        item->setText(QPoint(0, 0), text);
        return true;
    }
    auto tempList = element.attribute("textAttr").trimmed().split(" ");
    if (tempList.size() != 5)
    {
        return false;
    }
    /* 位置 */
    bool isOK1 = false;
    bool isOK2 = false;
    auto x = tempList[0].toDouble(&isOK1);
    auto y = tempList[1].toDouble(&isOK2);
    if (!(isOK1 & isOK2))
    {
        return false;
    }
    /* 模式 */
    auto mode = Optional<DrawTextMode>::emptyOptional();
    tempList[2] >> mode;
    if (!mode.success())
    {
        return false;
    }
    /* 原点 */
    auto spos = Optional<SourcePointPosition>::emptyOptional();
    tempList[3] >> spos;
    if (!spos.success())
    {
        return false;
    }
    auto wrap = QVariant(tempList[4]).toBool();
    item->setText(QPointF(x, y), text, mode.value(), spos.value(), wrap);
    return true;
}
bool GraphFactoryUtility::textToElement(const ItemBase* item, const GraphGlobal& /*cGlobal*/, QDomElement& element)
{
    Q_ASSERT(item != nullptr);
    auto text = item->text();
    if (text.isEmpty())
    {
        return true;
    }
    /* 文字属性的配置布局是 x y model spos warp */
    auto point = item->textStartPoint();
    auto model = item->drawTextMode();
    auto spp = item->textSourcePointPosition();
    auto modeStr = Optional<QString>::emptyOptional();
    model >> modeStr;
    if (!modeStr.success())
    {
        return false;
    }
    auto sppStr = Optional<QString>::emptyOptional();
    spp >> sppStr;
    if (!sppStr.success())
    {
        return false;
    }

    auto attr = QString("%1 %2 %3 %4 %5").arg(point.x()).arg(point.y()).arg(modeStr.value(), sppStr.value(), item->wrap() ? "true" : "false");
    element.setAttribute("text", text);
    element.setAttribute("textAttr", attr);
    return true;
}

void GraphFactoryUtility::brushFromElement(QBrush& brush, const QDomElement& element)
{
    /* brush="rgba style" */
    auto tempList = element.attribute(gGlobalXMLAttrBrush).split(" ");
    if (tempList.size() != 2)
    {
        auto color = QColor::fromRgb(0, 0, 0, 255);
        brush = QBrush(color, Qt::SolidPattern);
        return;
    }

    bool isOK = false;
    auto rgba = tempList[0].toUInt(&isOK, 16);
    if (!isOK)
    {
        rgba = 0xFF;
    }

    auto brushStyleType = QMetaEnum::fromType<Qt::BrushStyle>();
    auto stdStyleType = tempList[1].toStdString();
    auto style = Qt::BrushStyle(brushStyleType.keyToValue(stdStyleType.c_str(), &isOK));
    if (!isOK)
    {
        style = Qt::SolidPattern;
    }

    auto color = QColor::fromRgba(rgba);
    brush = QBrush(color, style);
}
void GraphFactoryUtility::brushToElement(const QBrush& brush, QDomElement& element)
{
    /* brush="rgba style" */
    auto brushStyleType = QMetaEnum::fromType<Qt::BrushStyle>();
    auto styleStr = brushStyleType.valueToKey(brush.style());
    if (styleStr == nullptr)
    {
        return;
    }
    auto tempAttr = QString("%1 %2").arg(styleStr).arg(quint64(brush.color().rgba64()), 0, 16);
    element.setAttribute(gGlobalXMLAttrBrush, tempAttr);
}
void GraphFactoryUtility::penFromElement(QPen& pen, const QDomElement& element)
{
    /* pen="rgba style width" */
    auto tempList = element.attribute(gGlobalXMLAttrPen).split(" ");
    if (tempList.size() != 3)
    {
        pen = QPen(Qt::SolidLine);
        auto color = QColor::fromRgb(0, 0, 0, 0xFF);
        pen.setColor(color);
        pen.setWidthF(1.0);
        return;
    }

    bool isOK = false;
    auto rgba = tempList[0].toUInt(&isOK, 16);
    if (!isOK)
    {
        rgba = 0xFF;
    }

    auto penStyleType = QMetaEnum::fromType<Qt::PenStyle>();
    auto stdStyleType = tempList[1].toStdString();
    auto style = Qt::PenStyle(penStyleType.keyToValue(stdStyleType.c_str(), &isOK));
    if (!isOK)
    {
        style = Qt::SolidLine;
    }

    auto width = tempList[2].toDouble(&isOK);
    if (!isOK)
    {
        width = 1.0;
    }
    pen = QPen(style);
    auto color = QColor::fromRgba(rgba);
    pen.setColor(color);
    pen.setWidthF(width);
}
void GraphFactoryUtility::penToElement(const QPen& pen, QDomElement& element)
{
    /* pen="rgba style width" */
    auto penStyleType = QMetaEnum::fromType<Qt::PenStyle>();
    auto styleStr = penStyleType.valueToKey(pen.style());
    if (styleStr == nullptr)
    {
        return;
    }
    auto tempAttr = QString("%1 %2 %3").arg(styleStr).arg(quint64(pen.color().rgba64()), 0, 16).arg(pen.widthF());
    element.setAttribute(gGlobalXMLAttrPen, tempAttr);
}
void GraphFactoryUtility::fontFromElement(ItemFontInfo& fontInfo, const QDomElement& element)
{
    fontInfo.custom = false;
    fontInfo.rgba = qRgba(0, 0, 0, 0xFF);
    fontInfo.strikeOut = false;
    fontInfo.underline = false;
    /* font="自定义字体 familie fontStyles pointSize 删除线 下划线 字体颜色" */
    auto tempList = element.attribute(gGlobalXMLAttrFont).split(" ");
    if (tempList.size() != 7)
    {
        return;
    }
    ItemFontInfo tempInfo;
    /* 自定义字体 */
    tempInfo.custom = QVariant(tempList[0]).toBool();
    tempInfo.name = tempList[1];
    tempInfo.fontStyles = tempList[2];
    bool isOK = false;
    tempInfo.size = tempList[3].toInt(&isOK, 10);
    if (!isOK)
    {
        return;
    }
    tempInfo.strikeOut = QVariant(tempList[4]).toBool();
    tempInfo.underline = QVariant(tempList[5]).toBool();
    tempInfo.rgba = tempList[6].toUInt(&isOK, 16);
    if (!isOK)
    {
        return;
    }
    fontInfo = tempInfo;
}
void GraphFactoryUtility::fontToElement(const ItemFontInfo& fontInfo, QDomElement& element)
{
    /* font="自定义字体 familie fontStyles pointSize 删除线 下划线 字体颜色" */
    auto tempAttr = QString("%1 %2 %3 %4 %5 %6 %7")
                        .arg(fontInfo.custom ? "true" : "false", fontInfo.name, fontInfo.fontStyles)
                        .arg(fontInfo.size)
                        .arg(fontInfo.strikeOut ? "true" : "false", fontInfo.underline ? "true" : "false")
                        .arg(fontInfo.rgba, 0, 16);
    element.setAttribute(gGlobalXMLAttrFont, tempAttr);
}
void GraphFactoryUtility::zFromElement(double& z, const QDomElement& element)
{
    bool isOK = false;
    z = element.attribute("z").trimmed().toDouble(&isOK);
    if (!isOK)
    {
        z = 0.0;
    }
}
void GraphFactoryUtility::zToElement(double z, QDomElement& element) { element.setAttribute("z", QString::number(z)); }

void GraphFactoryUtility::parseGlobal(GraphGlobal& graphGlobal, QDomElement& element)
{
    brushFromElement(graphGlobal.brush, element);
    penFromElement(graphGlobal.pen, element);
    fontFromElement(graphGlobal.fontInfo, element);
    zFromElement(graphGlobal.zAxis, element);

    gfInstallFont(graphGlobal.curDir, graphGlobal.fontInfo);
}
void GraphFactoryUtility::restoresGlobal(const GraphGlobal& graphGlobal, QDomElement& element)
{
    brushToElement(graphGlobal.brush, element);
    penToElement(graphGlobal.pen, element);
    fontToElement(graphGlobal.fontInfo, element);
    zToElement(graphGlobal.zAxis, element);
}

void GraphFactoryUtility::globalFromElement(ItemBase* item, const GraphGlobal& cGlobal, const QDomElement& element)
{
    Q_ASSERT(item != nullptr);
    auto brush = cGlobal.brush;
    if (element.hasAttribute(gGlobalXMLAttrBrush))
    {
        GraphFactoryUtility::brushFromElement(brush, element);
    }
    item->setBrush(brush);

    auto pen = cGlobal.pen;
    if (element.hasAttribute(gGlobalXMLAttrPen))
    {
        GraphFactoryUtility::penFromElement(pen, element);
    }
    item->setPen(pen);

    auto fontInfo = cGlobal.fontInfo;
    if (element.hasAttribute(gGlobalXMLAttrFont))
    {
        GraphFactoryUtility::fontFromElement(fontInfo, element);
        gfInstallFont(cGlobal.curDir, fontInfo);
    }
    item->setFontInfo(fontInfo);

    auto zAxis = cGlobal.zAxis;
    if (element.hasAttribute(gGlobalXMLAttrZ))
    {
        GraphFactoryUtility::zFromElement(zAxis, element);
    }
    item->setZValue(zAxis);
}

void GraphFactoryUtility::globalToElement(const ItemBase* item, const GraphGlobal& cGlobal, QDomElement& element)
{
    Q_ASSERT(item != nullptr);
    auto brush = item->brush();
    if (brush != cGlobal.brush)
    {
        brushToElement(brush, element);
    }
    auto pen = item->pen();
    if (pen != cGlobal.pen)
    {
        penToElement(pen, element);
    }
    auto fontInfo = item->fontInfo();

    if (fontInfo != cGlobal.fontInfo)
    {
        fontToElement(fontInfo, element);
    }
    auto zAsix = item->zValue();
    if (!qFuzzyCompare(1 + zAsix, 1 + cGlobal.zAxis))
    {
        zToElement(zAsix, element);
    }
}
