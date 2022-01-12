#ifndef NETSTATUSMONITOR_H
#define NETSTATUSMONITOR_H

#include <QDialog>

namespace Ui
{
    class NetStatusMonitor;
}
struct NetStatusItemInfo;
class NetStatusMonitorImpl;
class NetStatusMonitor : public QDialog
{
    Q_OBJECT

public:
    explicit NetStatusMonitor(QWidget* parent = nullptr);
    ~NetStatusMonitor();
    /* 通道数据改变 */
    void channelRecvDataChange(const NetStatusItemInfo&);
    void channelSendDataChange(const NetStatusItemInfo&);
    void timerEvent(QTimerEvent* event);

private:
    void resetUI();
    void showDataChannel(const QModelIndex& index);

private:
    Ui::NetStatusMonitor* ui;
    NetStatusMonitorImpl* m_impl;
};

#endif  // NETSTATUSMONITOR_H
