#ifndef STATUSBARMANAGERTHREAD_H
#define STATUSBARMANAGERTHREAD_H

#include "ProtocolXmlTypeDefine.h"
#include <QMap>
#include <QMutex>
#include <QThread>
#include <atomic>

struct StatusBarData
{
    QMap<DeviceID, QString> deviceIDOffName;  //离线设备  这个是用于任务流程控制界面，其它地方也能用，主要是为了处理一些不需要的设备而添加的
    QMap<DeviceID, QString> deviceIDSubName;  //分控设备  这个是用于任务流程控制界面，其它地方也能用，主要是为了处理一些不需要的设备而添加的

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
    void cczfSendDataSound();

private:
    std::atomic<bool> m_isRunning{ true };
    StatusBarData m_statusBarData;
    QString mCurProject;
};

#endif  // STATUSBARMANAGERTHREAD_H
