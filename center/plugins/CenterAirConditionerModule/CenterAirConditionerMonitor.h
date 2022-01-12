#ifndef CENTERAIRCONDITIONERMONITOR_H
#define CENTERAIRCONDITIONERMONITOR_H
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class CenterAirConditionerMonitor; }
QT_END_NAMESPACE

namespace cppmicroservices
{
class BundleContext;
}

class CenterAirConditionerMonitorImpl;
class CenterAirConditionerMonitor : public QWidget
{
    Q_OBJECT

public:
    CenterAirConditionerMonitor(cppmicroservices::BundleContext context, QWidget *parent = nullptr);
    ~CenterAirConditionerMonitor();
private:
    CenterAirConditionerMonitorImpl *m_impl;
    Ui::CenterAirConditionerMonitor *ui;
};
#endif // CENTERAIRCONDITIONERMONITOR_H
