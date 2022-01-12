#include "NameLabel.h"

#include <QHBoxLayout>

NameLabel::NameLabel(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    initLayout();

    parseNode(domEle);

    setText(mNameLableInfo.text);

    setStyleSheet("QLabel {"
                  "color: #386487;"
                  "}");
}

void NameLabel::setText(const QString& text) { mLabel->setText(text); }

QString NameLabel::getText() const { return mLabel->text(); }

void NameLabel::initLayout()
{
    mLabel = new QLabel(this);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mLabel);
}

void NameLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    mNameLableInfo.text = domEle.attribute("text");

    // 文本对齐方式
    if (domEle.hasAttribute("Align"))
    {
        auto align = domEle.attribute("Align");
        if (align == "Left")
        {
            mLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (align == "Right")
        {
            mLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (align == "Center")
        {
            mLabel->setAlignment(Qt::AlignCenter);
        }
    }
    else
    {
        mLabel->setAlignment(Qt::AlignCenter);
    }
}
