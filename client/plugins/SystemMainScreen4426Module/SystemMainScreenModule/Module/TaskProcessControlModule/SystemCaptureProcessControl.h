#ifndef SYSTEMCAPTUREPROCESSCONTROL_H
#define SYSTEMCAPTUREPROCESSCONTROL_H

#include "DeviceProcessMessageDefine.h"
#include "SystemCaptureProcessCmd.h"
#include "SystemWorkMode.h"

#include <ExtendedConfig.h>
#include <QWidget>

class QComboBox;

namespace Ui
{
    class SystemCaptureProcessControl;
}

class SystemCaptureProcessControlImpl;
class SystemCaptureProcessControl : public QWidget
{
    Q_OBJECT

public:
    explicit SystemCaptureProcessControl(QWidget* parent = nullptr);
    ~SystemCaptureProcessControl() override;

    void setShowWidget(SystemWorkMode workMode);

    void setCurDeviceComBox();

    void setXMLSlot();

    int getCurrentDeviceId();

    int getCurrentModeID();

    void resetUIData();

signals:
    void signalsCmdDeviceJson(const QString& json);
    void signalsUnitDeviceJson(const QString& json);

private:
    // 设备是否在线检测
    bool deviceIsOnline(const DeviceID& deviceID);
    // 设备模式检测
    bool deviceIsMode(const DeviceID& deviceID, int curModeId);
    // 设备本分控检测
    bool deviceIsSelfControl(const DeviceID& deviceID);
    // 判断是否是主机
    bool deviceIsMaster(const DeviceID& deviceID);

private slots:
    void slotCmdRequest(const CmdRequest& cmdRequest, bool isCheckMasterSlave);
    void slotUnitParamRequest(const UnitParamRequest& unitParamRequest, bool isCheckMasterSlave);
    void slotAutoManualBtnSwitch(int id);

public:
    void timerEvent(QTimerEvent* event) override;

private:
    Ui::SystemCaptureProcessControl* ui;
    SystemCaptureProcessControlImpl* m_impl;
};

#endif  // SYSTEMCAPTUREPROCESSCONTROL_H
