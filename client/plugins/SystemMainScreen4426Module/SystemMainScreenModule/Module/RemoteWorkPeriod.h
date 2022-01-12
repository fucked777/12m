#ifndef REMOTEWORKPERIOD_H
#define REMOTEWORKPERIOD_H

#include "ProtocolXmlTypeDefine.h"
#include <QDialog>

namespace Ui
{
    class RemoteWorkPeriod;
}

class RemoteWorkPeriod : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteWorkPeriod(QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);
    ~RemoteWorkPeriod();

signals:
    void signalsCmdDeviceJson(QString json);
    void signalsUnitDeviceJson(QString json);

protected slots:
    void btnClicked();

private:
    void init();
    bool targetNumCheck(const int deviceId, const int curTargetNum);

    // 设备是否在线检测
    bool deviceIsOnline();
    // 设备模式检测
    bool deviceIsMode();
    // 设备本分控检测
    bool deviceIsSelfControl();

    // 判断是否是主机
    bool deviceIsMaster();

private:
    Ui::RemoteWorkPeriod* ui;
    int d_deviceId;
    int d_modeId;
};

#endif  // REMOTEWORKPERIOD_H
