#ifndef DOPPLERSET_H
#define DOPPLERSET_H

#include <QDialog>

namespace Ui
{
    class DopplerSet;
}

class DopplerSet : public QDialog
{
    Q_OBJECT

public:
    explicit DopplerSet(QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);
    ~DopplerSet();

protected slots:
    void btnSetClicked();

signals:
    void signalsCmdDeviceJson(const QString& json);

private:
    void init();
    bool subControlCheck(const int deviceId, const int unitId, const QString& paramId = "ControlMode", int subControlVal = 1);
    bool workModeCheck(int srcDeviceId, const int curWorkMode);
    bool serverCheck();

private:
    Ui::DopplerSet* ui;
    int d_deviceId;
    int d_modeId;
};

#endif  // DOPPLERSET_H
