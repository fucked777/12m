#include "GroupBox.h"

#include <QGroupBox>
#include <QVBoxLayout>

#include "MainScreenControlFactory.h"

GroupBox::GroupBox(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
{
    initLayout();

    parseNode(domEle);

    setTitle(mGroupBoxInfo.text);

    setStyleSheet("QGroupBox{"
                  "font: 11pt;"
                  "border:1px solid #C0DCF2;"
                  "border-radius:5px;"
                  "margin-top:5px;"
                  "}"
                  "QGroupBox::title{"
                  "color: #386487;"
                  "subcontrol-origin:margin;"
                  "subcontrol-position :top left;"
                  "position:5 5px;"
                  "font-size:18pt;"
                  "left:10px;"
                  "}");
}

void GroupBox::setTitle(const QString& title) { mGroupBox->setTitle(title); }

QString GroupBox::getTitle() const { return mGroupBox->title(); }

void GroupBox::initLayout()
{
    mGroupBox = new QGroupBox(this);
    mGroupBox->setContentsMargins(0, 0, 0, 0);

    auto groupBoxVLayout = new QVBoxLayout;
    mGroupBox->setLayout(groupBoxVLayout);

    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);

    vLayout->addWidget(mGroupBox);
}

void GroupBox::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    mGroupBoxInfo.text = domEle.attribute("text");

    auto groupBoxVLayout = qobject_cast<QVBoxLayout*>(mGroupBox->layout());

    QDomNode domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        QDomElement subDomElement = domNode.toElement();
        if (!subDomElement.isNull())
        {
            QString tagName = subDomElement.tagName();

            QWidget* widget = nullptr;
            widget = MainScreenControlFactory::createControl(tagName, subDomElement, this);

            if (widget != nullptr)
            {
                groupBoxVLayout->addWidget(widget);
            }
        }
        domNode = domNode.nextSibling();
    }
}
