#ifndef TASKPLANWIDGET_H
#define TASKPLANWIDGET_H

#include <QWidget>

#include "TaskPlanMessageDefine.h"

namespace Ui
{
    class TaskPlanWidget;
}

class TaskPlanTableModel;

// 任务计划模块
class TaskPlanWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskPlanWidget(QWidget* parent = nullptr);
    ~TaskPlanWidget();

private:
    // 重置任务计划信息
    void resetTaskPlanInfo();

private slots:
    // 读取任务运行流程数据
    void slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data);
    // 更新任务运行信息
    void slotUpdateTaskInfo(const DeviceWorkTask& deviceWorkTask);

private:
    Ui::TaskPlanWidget* ui;

    TaskPlanTableModel* mTaskPlanTableModel = nullptr;
};

#endif  // TASKPLANWIDGET_H
