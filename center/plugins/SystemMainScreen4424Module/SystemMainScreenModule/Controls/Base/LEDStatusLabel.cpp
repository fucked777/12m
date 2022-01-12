#include "LEDStatusLabel.h"

#include <QHBoxLayout>

static constexpr const char* redQss = "QLabel {"
                                      "background-image: url(:/image/red.png);"
                                      "background-position:center;"
                                      "background-repeat:no-repeat;"
                                      "border-style: none;"
                                      "}";

static constexpr const char* greenQss = "QLabel {"
                                        "background-image: url(:/image/green.png);"
                                        "background-position:center;"
                                        "background-repeat:no-repeat;"
                                        "border-style:none;"
                                        "}";

static constexpr const char* grayQss = "QLabel {"
                                       "background-image: url(:/image/gray.png);"
                                       "background-position:center;"
                                       "background-repeat:no-repeat;"
                                       "border-style:none;"
                                       "}";

LEDStatusLabel::LEDStatusLabel(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
    , StatusController()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    initLayout();

    setMinimumSize(28, 28);
    setStyleSheet(grayQss);

    parseNode(domEle);
}

void LEDStatusLabel::setValue(const QVariant& value)
{
    auto ledItem = mLedItemMap[value];
    setStatus(ledItem.status);
    setToolTip(ledItem.toolTip);
}

QVariant LEDStatusLabel::getValue() const { return QVariant(); }

void LEDStatusLabel::statusChanged(StatusController::Status status)
{
    switch (status)
    {
    case Unknown:
    {
        setStyleSheet(grayQss);
    }
    break;
    case Normal: setStyleSheet(greenQss); break;
    case Abnormal: setStyleSheet(redQss); break;
    default: setStyleSheet(grayQss); break;
    }
}

void LEDStatusLabel::initLayout()
{
    mLabel = new QLabel(this);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mLabel);
}

void LEDStatusLabel::parseNode(const QDomElement& domEle)
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
            LEDItem ledItem;
            ledItem.value = subDomEle.attribute("value");
            ledItem.toolTip = subDomEle.attribute("toolTip", "");

            // 颜色
            auto ledColor = subDomEle.attribute("ledColor");
            Status status = Unknown;
            if (ledColor.toUpper() == "GREEN")
            {
                status = Normal;
            }
            else if (ledColor.toUpper() == "RED")
            {
                status = Abnormal;
            }

            ledItem.status = status;

            mLedItemMap[ledItem.value] = ledItem;
        }

        domNode = domNode.nextSibling();
    }
}
