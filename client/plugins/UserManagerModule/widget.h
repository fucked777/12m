#ifndef WIDGET_H
#define WIDGET_H
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

namespace cppmicroservices
{
class BundleContext;
}

class WidgetImpl;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(cppmicroservices::BundleContext context, QWidget *parent = nullptr);
    ~Widget();
private:
    WidgetImpl *m_impl;
    Ui::Widget *ui;
};
#endif // WIDGET_H
