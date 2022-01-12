#include "widget.h"
#include "CppMicroServicesUtility.h"
#include "ui_widget.h"

class WidgetImpl
{
public:
    cppmicroservices::BundleContext context;
};
Widget::Widget(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , m_impl(new WidgetImpl)
    , ui(new Ui::Widget)
{
    m_impl->context = context;
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
    delete m_impl;
}
