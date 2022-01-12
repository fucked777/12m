#include "ComboBoxStatusLabel.h"

ComboBoxStatusLabel::ComboBoxStatusLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
{
    parseNode(domEle);
}

void ComboBoxStatusLabel::setValue(const QVariant& value)
{
    qDebug() << "ComboBoxStatusLabel";
    auto text = mEnumMap[value];

    if (!text.isEmpty())
    {
        setText(text);
        setStatus(Normal);
    }
    else
    {
        setText(getDefaultText());
    }
}

QVariant ComboBoxStatusLabel::getValue() const { return QVariant(); }

void ComboBoxStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        auto tagName = subDomEle.tagName();
        if (tagName == "Item")
        {
            mEnumMap[subDomEle.attribute("value")] = subDomEle.attribute("text");
        }

        domNode = domNode.nextSibling();
    }
}
