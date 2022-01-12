#include "TaskPlanWidget.h"
#include "ui_TaskPlanWidget.h"

#include "GlobalData.h"
#include "MessagePublish.h"
#include "SatelliteManagementDefine.h"
#include "SubscriberHelper.h"
#include "TaskPlanMessageSerialize.h"

TaskPlanWidget::TaskPlanWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TaskPlanWidget)
{
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData");
    qRegisterMetaType<CurrentRunningTaskPlanData>("const CurrentRunningTaskPlanData&");
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData&");
    ui->setupUi(this);

    setStyleSheet("QLabel {"
                  "color: #386487;"
                  "}");

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskStepStatusReadable, this,
            &TaskPlanWidget::slotReadAutoRunTaskStepStatus);
}

TaskPlanWidget::~TaskPlanWidget() { delete ui; }

void TaskPlanWidget::resetTaskPlanInfo()
{
    ui->planTypeLabel->setText("-");
    ui->taskCodeLabel->setText("-");
    ui->systemWorkModeLabel->setText("-");
    ui->taskIdentityLabel->setText("-");
    ui->systemWorkWayLabel->setText("-");
    ui->equipmentNumLabel->setText("-");
    ui->taskStartTimeLabel->setText("-");
    ui->taskEndTimeLabel->setText("-");
    ui->taskStatusLabel->setText("-");
}

void TaskPlanWidget::slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data)
{
    // 更新当前运行任务的信息
    slotUpdateTaskInfo(data.taskPlanData.deviceWorkTask);
}

void TaskPlanWidget::slotUpdateTaskInfo(const DeviceWorkTask& deviceWorkTask)
{
    DeviceWorkTaskTarget mainTarget;
    // 有效
    if (deviceWorkTask.getMainTarget(mainTarget))
    {
        ui->planTypeLabel->setText(TaskPlanHelper::taskTypeToString(mainTarget.m_planType));
        ui->taskCodeLabel->setText(mainTarget.m_task_code);
        //    ui->systemWorkModeLabel->setText();

        SatelliteManagementData satelliteData;
        if (GlobalData::getSatelliteManagementData(mainTarget.m_task_code, satelliteData))
        {
            ui->taskIdentityLabel->setText(satelliteData.m_satelliteIdentification);
        }
        ui->systemWorkWayLabel->setText(TaskPlanHelper::planWorkModeToDesc(deviceWorkTask.m_working_mode));
        //    ui->equipmentNumLabel->setText();
        ui->taskStartTimeLabel->setText(deviceWorkTask.m_startTime.toString(DATETIME_DISPLAY_FORMAT2));
        ui->taskEndTimeLabel->setText(deviceWorkTask.m_endTime.toString(DATETIME_DISPLAY_FORMAT2));
        ui->taskStatusLabel->setText(TaskPlanHelper::taskStatusToString(deviceWorkTask.m_task_status));
    }
    else
    {
        resetTaskPlanInfo();  // 无效
    }
}
