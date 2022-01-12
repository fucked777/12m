#ifndef WIDGET_H
#define WIDGET_H

#include "testInterface.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void showPack(const ProtocolPack& pack);

private:
    Ui::Widget* ui;
    TestInterface demo;
};
#endif  // WIDGET_H
