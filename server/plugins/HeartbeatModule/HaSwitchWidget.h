#ifndef HASWITCHWIDGET_H
#define HASWITCHWIDGET_H

#include <QProcess>
#include <QWidget>

namespace Ui
{
    class HaSwitchWidget;
}
namespace cppmicroservices
{
    class BundleContext;
}

class HeartbeatThread;
class HaSwitchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HaSwitchWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~HaSwitchWidget();

    void timerEvent(QTimerEvent* event);

private:
    void changeMaster();
    void changeSlave();

private:
    Ui::HaSwitchWidget* ui;
    bool m_isMaster{ false }; /* 当前是主机还是备机 */
    int m_timerID{ -1 };
    QString m_masterToSlave;  //主机切备机脚本
    QString m_slaveToMaster;  //备机切主机脚本
};

#endif  // HASWITCHWIDGET_H
