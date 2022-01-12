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
    slotUpdateTaskInfo(data);
}

void TaskPlanWidget::slotUpdateTaskInfo(const CurrentRunningTaskPlanData& data)
{
    const auto& deviceWorkTask = data.taskPlanData.deviceWorkTask;

    DeviceWorkTaskTarget mainTarget;
    // 任务计划有效
    if (deviceWorkTask.getMainTarget(mainTarget))
    {
        ui->planTypeLabel->setText(TaskPlanHelper::taskTypeToString(mainTarget.m_planType));  // 计划类型
        ui->taskCodeLabel->setText(mainTarget.m_task_code);                                   // 任务代号

        // 获取主跟目标工作模式
        auto list = deviceWorkTask.getMainTargetWorkMode();
        QStringList workModeList;
        for (auto workMode : list)
        {
            workModeList << SystemWorkModeHelper::systemWorkModeToDesc(workMode);
        }
        ui->systemWorkModeLabel->setText(workModeList.join("+"));  // 系统工作模式

        // 获取卫星标识
        SatelliteManagementData satelliteData;
        if (GlobalData::getSatelliteManagementData(mainTarget.m_task_code, satelliteData))
        {
            ui->taskIdentityLabel->setText(satelliteData.m_satelliteIdentification);  // 卫星标识
        }
        ui->systemWorkWayLabel->setText(TaskPlanHelper::planWorkModeToDesc(deviceWorkTask.m_working_mode));  // 系统工作方式
        ui->equipmentNumLabel->setText(QString::number(data.msg.devNumber, 16));                             // 设备组合号
        ui->taskStartTimeLabel->setText(deviceWorkTask.m_startTime.toString(DATETIME_DISPLAY_FORMAT2));      // 任务开始时间
        ui->taskEndTimeLabel->setText(deviceWorkTask.m_endTime.toString(DATETIME_DISPLAY_FORMAT2));          // 任务结束时间
        ui->taskStatusLabel->setText(TaskPlanHelper::taskStatusToString(deviceWorkTask.m_task_status));      // 任务状态
    }
    else if (!data.msg.linkLineMap.isEmpty())  // 手动任务控制流程，手动下发参数宏、配置宏时
    {
        const auto& manualMessage = data.msg;

        // 如果有多种模式，优先判断有不有测控模式
        SystemWorkMode workMode = NotDefine;
        int ck = 0;
        int sc = 0;
        QSet<QString> workModeSet;
        for (auto& item : manualMessage.linkLineMap)
        {
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(item.workMode))
            {
                workMode = item.workMode;
                ++ck;
            }
            if (SystemWorkModeHelper::isDataTransmissionWorkMode(item.workMode))
            {
                ++sc;
            }
            workModeSet << SystemWorkModeHelper::systemWorkModeToDesc(item.workMode);
        }

        if (ck > 0 && sc > 0)
        {
            ui->planTypeLabel->setText(TaskPlanHelper::taskTypeToString(TaskPlanType::MeasureCtrlDataTrasn));
        }
        else if (ck > 0)
        {
            ui->planTypeLabel->setText(TaskPlanHelper::taskTypeToString(TaskPlanType::MeasureControl));
        }
        else if (sc > 0)
        {
            ui->planTypeLabel->setText(TaskPlanHelper::taskTypeToString(TaskPlanType::DataTrans));
        }
        else
        {
            ui->planTypeLabel->setText("-");
        }

        // 有测控模式就使用测控模式信息，否则就默认使用第一个模式的信息
        LinkLine linkLine;
        if (workMode != NotDefine)
        {
            linkLine = manualMessage.linkLineMap[workMode];
        }
        else
        {
            linkLine = manualMessage.linkLineMap.first();
        }
        auto mainTarget = linkLine.targetMap[linkLine.masterTargetNo];

        // 获取卫星标识
        SatelliteManagementData satelliteData;
        if (GlobalData::getSatelliteManagementData(mainTarget.taskCode, satelliteData))
        {
            ui->taskIdentityLabel->setText(satelliteData.m_satelliteIdentification);  // 卫星标识
        }

        ui->equipmentNumLabel->setText(QString::number(manualMessage.devNumber, 16));  // 设备组合号
        ui->systemWorkModeLabel->setText(workModeSet.toList().join("+"));
        ui->taskCodeLabel->setText(linkLine.targetMap.first().taskCode);
    }
    else
    {
        resetTaskPlanInfo();  // 无效
    }
}
