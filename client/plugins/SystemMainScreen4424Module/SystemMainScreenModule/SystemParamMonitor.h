#ifndef SYSTEMPARAMMONITOR_H
#define SYSTEMPARAMMONITOR_H

#include <QMainWindow>

class QDockWidget;
class SystemParamMonitorImpl;
class SystemParamMonitor : public QMainWindow
{
    Q_OBJECT
public:
    explicit SystemParamMonitor(QWidget* parent = nullptr);
    ~SystemParamMonitor();

    void initWindowState();
    void saveWindowState();

protected:
    void showEvent(QShowEvent* event) override;

signals:
    void signalsCmdDeviceJson(QString json);
    void signalsUnitDeviceJson(QString json);

private:
    void initDeviceParamWidget();
    void initSystemLogWidget();
    void initAutoTaskWidget();
    void initTaskPlanWidget();
    void initTimeWidget();

private:
    SystemParamMonitorImpl* mImpl = nullptr;
};

#endif  // SYSTEMPARAMMONITOR_H
