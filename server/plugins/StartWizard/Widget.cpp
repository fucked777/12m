#include "Widget.h"
#include "PlatformDefine.h"
#include "ui_Widget.h"

Widget::Widget(QWidget* parent)
    : IStartWizard(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(this, &Widget::sg_test,
            [=](bool a, bool b, QLocalSocket::LocalSocketState s) { ui->textEdit->append(QString("%1--%2--%3").arg(a).arg(b).arg(s)); });
}

Widget::~Widget() { delete ui; }

void Widget::serviceInstallStatusChange(const ModuleInstallInfo& info)
{
    ui->textEdit->append(QString("%1--%2--%3").arg(info.name, ModuleInstallStatusHelper::toDesc(info.status)).arg(count));
}
