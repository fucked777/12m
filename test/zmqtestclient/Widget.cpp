#include "Widget.h"
#include "ui_Widget.h"

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    demo.setSrcAddr("demo");
    QString msg;
    demo.setConnectAddr("tcp://192.168.0.113:5555");
    if (!demo.startServer(&msg))
    {
        qWarning() << msg;
    }
    connect(&demo, &TestInterface::sg_testPack, this, &Widget::showPack);
}
void Widget::showPack(const ProtocolPack& pack)
{
    ui->send_data_2->setText(pack.data);
    ui->send_des_2->setText(pack.desID);
    ui->send_module_2->setChecked(pack.module);
    ui->send_opt_2->setText(pack.operation);
    ui->send_ack_2->setText(pack.operationACK);
    ui->send_src_2->setText(pack.srcID);
}
Widget::~Widget() { delete ui; }

void Widget::on_pushButton_clicked()
{
    ProtocolPack pack;
    pack.data = ui->send_data->text().toUtf8();
    pack.desID = ui->send_des->text();
    pack.module = ui->send_module->isChecked();
    pack.operation = ui->send_opt->text();
    pack.operationACK = ui->send_ack->text();
    // pack.srcID = ui->send_src->text();
    demo.send(pack);
}
