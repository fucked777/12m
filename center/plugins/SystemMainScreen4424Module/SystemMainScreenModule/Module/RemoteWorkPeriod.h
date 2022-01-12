#ifndef REMOTEWORKPERIOD_H
#define REMOTEWORKPERIOD_H

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
    bool subControlCheck(const int deviceId, const int unitId, const QString& paramId = "ControlMode", int subControlVal = 1);
    bool workModeCheck(int srcDeviceId, const int curWorkMode);
    bool serverCheck();
    bool targetNumCheck(const int deviceId, const int curTargetNum);

private:
    Ui::RemoteWorkPeriod* ui;
    int d_deviceId;
    int d_modeId;
};

#endif  // REMOTEWORKPERIOD_H
