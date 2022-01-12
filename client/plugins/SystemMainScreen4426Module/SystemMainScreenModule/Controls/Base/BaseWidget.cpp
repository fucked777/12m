#include "BaseWidget.h"

#include "MainScreenControlFactory.h"

#include <QPainter>
#include <QStyleOption>
#include <QVariant>

class BaseWidgetImpl
{
public:
    bool isShow{ false };
    QSize minSize;
};

BaseWidget::BaseWidget(const QDomElement& domEle, QWidget* parent)
    : QWidget(parent)
    , mImpl(new BaseWidgetImpl())
{
    parseNode(domEle);
}

BaseWidget::~BaseWidget()
{
    if (mImpl != nullptr)
    {
        delete mImpl;
        mImpl = nullptr;
    }
}

void BaseWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void BaseWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)
    if (!mImpl->isShow)
    {
        return;
    }
}

void BaseWidget::showEvent(QShowEvent* event)
{
    mImpl->isShow = true;
    QWidget::showEvent(event);
}

void BaseWidget::hideEvent(QHideEvent* event)
{
    mImpl->isShow = false;
    QWidget::hideEvent(event);
}

void BaseWidget::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    if (domEle.hasAttribute("minSize"))
    {
        auto sizeList = domEle.attribute("minSize").trimmed().split(",");
        if (sizeList.size() == 2)
        {
            mImpl->minSize = QSize(sizeList.at(0).trimmed().toInt(), sizeList.at(1).trimmed().toInt());
            setMinimumSize(mImpl->minSize);
        }
    }
}
