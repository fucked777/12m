#include "DockWidget.h"
#include <QPainter>
#include <QStyleOption>

DockWidget::DockWidget(QWidget* parent)
    : QDockWidget(parent)
{
}

DockWidget::~DockWidget() {}

void DockWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}
