#ifndef SYSTEMMAINSCREEN_H
#define SYSTEMMAINSCREEN_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SystemMainScreen;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}

struct TimeConfigInfo;
class SystemMainScreenImpl;
class SystemMainScreen : public QMainWindow
{
    Q_OBJECT

public:
    SystemMainScreen(QWidget* parent = nullptr);
    ~SystemMainScreen();

signals:
    void signalTimeConfigChanged(const TimeConfigInfo& info);

private:
    void initSystemLogWidget();
    void initTaskPlanWidget();
    void initTimeWidget();

private:
    Ui::SystemMainScreen* ui;
};

#endif  // SYSTEMMAINSCREEN_H
