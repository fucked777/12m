#include "NumberStatusLabel.h"

NumberStatusLabel::NumberStatusLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
{
    parseNode(domEle);
}

void NumberStatusLabel::setValue(const QVariant& value)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    auto valueString = QString("%1").arg(value.toDouble(), 0, 'f', 3).replace(rx, "");
    setText(valueString);
    setStatus(Normal);
}

QVariant NumberStatusLabel::getValue() const { return QVariant(); }

void NumberStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
}
