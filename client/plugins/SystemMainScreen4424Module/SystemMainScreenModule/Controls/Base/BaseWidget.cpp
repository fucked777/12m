#include "BaseWidget.h"

#include "MainScreenControlFactory.h"

#include <QPainter>
#include <QStyleOption>
#include <QVariant>

BaseWidget::BaseWidget(const QDomElement& domEle, QWidget* parent)
    : QWidget(parent)
{
}

BaseWidget::~BaseWidget() {}

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
    if (!mIsShow)
    {
        return;
    }
}

void BaseWidget::showEvent(QShowEvent* event)
{
    mIsShow = true;
    QWidget::showEvent(event);
}

void BaseWidget::hideEvent(QHideEvent* event)
{
    mIsShow = false;
    QWidget::hideEvent(event);
}
