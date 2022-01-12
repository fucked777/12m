#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>

namespace Ui
{
    class TimeWidget;
}

struct DeviceWorkTask;
struct CurrentRunningTaskPlanData;
// 时间模块
class TimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeWidget(QWidget* parent = nullptr);
    ~TimeWidget();

protected:
    void timerEvent(QTimerEvent* event);
    virtual void paintEvent(QPaintEvent* event) override;

private slots:
    // 读取任务运行流程数据
    void slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data);
    // 更新当前任务时间信息
    void slotUpdateTaskTime(const DeviceWorkTask& deviceWorkTask);

private:
    Ui::TimeWidget* ui;
};

#endif  // TIMEWIDGET_H
