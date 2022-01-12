#ifndef SYSTEMMAINSCREEN_H
#define SYSTEMMAINSCREEN_H

#include "DeviceDataLoadThread.h"
#include <QMainWindow>
#include <QWidget>
namespace Ui
{
    class SystemMainScreen;
}

struct ControlCmdResponse;
struct CmdResult;
class AntennaControlWidget4424;
class AntennaControlWidget4426;
class TaskProcessControlWidget;
class SystemMainScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SystemMainScreen(QWidget* parent = nullptr);
    ~SystemMainScreen();

signals:
    //链路配置发令
    void signalManualFunction(QString json);

    void signalsCmdDeviceJson(QString json);

    void signalsUnitDeviceJson(QString json);

    void signalsDeviceCMDResult(const QByteArray& data);

    /*分机过程控制命令响应 */
    void signalsCMDResponceJson(const QByteArray& data);

public slots:
    void slotsControlCmdResponse(const ControlCmdResponse& cmdResponce);

private:
    void init();

private:
    Ui::SystemMainScreen* ui;

    AntennaControlWidget4424* mAntennaControlWidget4424 = nullptr;
    AntennaControlWidget4426* mAntennaControlWidget4426 = nullptr;
    TaskProcessControlWidget* mTaskProcessControlWidget = nullptr;
    QString m_CurProject = "4424";
    DeviceDataLoadThread* m_deviceDataLoadThread = nullptr;
};

#endif  // SYSTEMMAINSCREEN_H
