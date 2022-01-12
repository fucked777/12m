#ifndef STATUSBARMANAGERTHREAD_H
#define STATUSBARMANAGERTHREAD_H

#include <QMutex>
#include <QThread>
#include <atomic>

struct StatusBarData
{
    QList<QString> offlineDeviceList;  // 离线设备

    QList<QString> subCtrlDeviceList;  // 分控设备

    QString serverMasterText;  // 服务器主备显示文本

    bool SZeroCalibrationOn = false;   // 4426S校零变频器是否加电
    bool KaZeroCalibrationOn = false;  // 4426Ka校零变频器是否加电
};

class StatusBarManagerThread : public QThread
{
    Q_OBJECT
public:
    StatusBarManagerThread();
    void stopRunning();

signals:
    void signalRefreshUI(const StatusBarData&);

protected:
    void run() override;

private:
    std::atomic<bool> m_isRunning{ true };
    StatusBarData m_statusBarData;
    QString mCurProject;
};

#endif  // STATUSBARMANAGERTHREAD_H
