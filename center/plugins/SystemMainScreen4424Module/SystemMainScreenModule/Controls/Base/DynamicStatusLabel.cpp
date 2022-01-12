#include "DynamicStatusLabel.h"

DynamicStatusLabel::DynamicStatusLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
    , d_dealType("")
{
    parseNode(domEle);
}

void DynamicStatusLabel::setValue(const QVariant& value)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    auto valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");
    if (value == "-1")
    {
        setText("变频");
    }
    else
    {
        setText(valueString);
    }
    setStatus(Normal);
}

QVariant DynamicStatusLabel::getValue() const { return QVariant(); }

void DynamicStatusLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString DynamicStatusLabel::getDealType() const { return d_dealType; }

void DynamicStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    d_dealType = domEle.attribute("dealType");
}
