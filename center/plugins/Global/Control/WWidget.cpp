
#include "WWidget.h"

WWidget::WWidget(QWidget* parent)
    : QWidget(parent)
{
}

void WWidget::resetValue() {}
void WWidget::setMinNum(const QVariant& /* minNum */) {}
void WWidget::setMaxNum(const QVariant& /* maxNum */) {}
void WWidget::setNumRange(const QVariant& /* minNum */, const QVariant& /* maxNum */) {}
void WWidget::setParamStatus(DataBuilder::FormsStatus /* status */) {}
void WWidget::setName(const QString&) {}
void WWidget::setUnit(const QString&) {}
void WWidget::setDecimal(int /* dec */) {}
void WWidget::setMaxLength(int /* len */) {}
void WWidget::setAttr(const ParameterAttribute& /* attr */, const QMap<QString, QVariant>& /* map */) {}

void WWidget::setAttrOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map) {}
void WWidget::setNameWidth(int) {}
void WWidget::setValueWidth(int) {}
void WWidget::setControlHeight(int) {}
void WWidget::addItem(const QVariant&, const QVariant&) {}
void WWidget::setItem(const QMap<QString, QVariant>&) {}

void WWidget::setItemOrder(const QMap<int, QPair<QString, QVariant>>&) {}

// void WWidget::setEnabled(bool) {}
void WWidget::setReadOnly(bool) {}

WWidget::~WWidget() {}
void WWidget::setValueStyleSheet(const QString&) {}

void WWidget::setParamDesc(const QString&) {}

void WWidgetEmpty::setValue(const QVariant& /* value */) {}
QVariant WWidgetEmpty::value() { return QVariant(); }
QString WWidget::text() { return QString(); }
QVariant WWidget::value(bool& isValid)
{
    isValid = false;
    return QVariant();
}
WWidgetEmpty::WWidgetEmpty(QWidget* parent)
    : WWidget(parent)
{
}
WWidgetEmpty::~WWidgetEmpty() {}

void WWidget::outOfRangesLineEdit(QWidget* widget, bool value)
{
    if (value)
    {
        widget->setStyleSheet("QLineEdit{border:1px solid red }");
    }
    else
    {
        // LineEdit恢复之前的状态
        widget->setStyleSheet("QLineEdit{border:1px solid gray border-radius:1px}");
    }
}
