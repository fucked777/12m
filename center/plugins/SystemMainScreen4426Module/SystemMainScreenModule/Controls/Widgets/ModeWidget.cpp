#include "ModeWidget.h"

#include <QVBoxLayout>

#include "MainScreenControlFactory.h"

ModeWidget::ModeWidget(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
{
    initLayout();

    parseNode(domEle);
}

ModeWidget::~ModeWidget() {}

int ModeWidget::getModeId() const { return mModeId; }

QString ModeWidget::getModeName() const { return mModeName; }

void ModeWidget::initLayout()
{
    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);
}

void ModeWidget::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
    mModeId = domEle.attribute("modeId").toInt(nullptr, 16);
    mModeName = domEle.attribute("name");

    auto vLayout = layout();

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomElement = domNode.toElement();
        if (!subDomElement.isNull())
        {
            auto tagName = subDomElement.tagName();

            auto widget = MainScreenControlFactory::createControl(tagName, subDomElement);
            if (widget != nullptr)
            {
                vLayout->addWidget(widget);
            }
        }
        domNode = domNode.nextSibling();
    }
}
