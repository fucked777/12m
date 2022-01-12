#include "AutoTaskWidget.h"
#include "ui_AutoTaskWidget.h"

#include <QMessageBox>

#include "GlobalData.h"
#include "MessagePublish.h"
#include "SubscriberHelper.h"
#include "TaskPlanMessageSerialize.h"

AutoTaskWidget::AutoTaskWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AutoTaskWidget)
{
    ui->setupUi(this);

    setStyleSheet("QLabel {"
                  "color: #386487;"
                  "}");

    initPixmap();
    init();

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskStepStatusReadable, this,
            &AutoTaskWidget::slotReadAutoRunTaskStepStatus);

    startTimer(1000);
}

AutoTaskWidget::~AutoTaskWidget() { delete ui; }

void AutoTaskWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)

    // 获取任务运行控制类型
    if (GlobalData::getAutoRunTaskFlag())
    {
        ui->autoRunRadioBtn->setChecked(true);
    }
    else
    {
        ui->manualRunRadioBtn->setChecked(true);
    }
}

void AutoTaskWidget::init()
{
    mBtnGroup.addButton(ui->manualRunRadioBtn, 0);
    mBtnGroup.addButton(ui->autoRunRadioBtn, 1);

    connect(&mBtnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this,
            &AutoTaskWidget::slotShowSwitchAutoRunDialog);
}

void AutoTaskWidget::initPixmap()
{
    mTaskPreparationGrayPixmap = QPixmap(":/image/task_preparation_gray.png");
    mTaskPreparationGreenPixmap = QPixmap(":/image/task_preparation_green.png");
    mTaskPreparationRedPixmap = QPixmap(":/image/task_preparation_red.png");

    mZeroCorrectionGrayPixmap = QPixmap(":/image/zero_correction_gray.png");
    mZeroCorrectionGreenPixmap = QPixmap(":/image/zero_correction_green.png");
    mZeroCorrectionRedPixmap = QPixmap(":/image/zero_correction_red.png");

    mTurnToWaitingPointGrayPixmap = QPixmap(":/image/turn_to_waiting_point_gray.png");
    mTurnToWaitingPointGreenPixmap = QPixmap(":/image/turn_to_waiting_point_green.png");
    mTurnToWaitingPointRedPixmap = QPixmap(":/image/turn_to_waiting_point_red.png");

    mPhaseCorrectionGrayPixmap = QPixmap(":/image/phase_correction_gray.png");
    mPhaseCorrectionGreenPixmap = QPixmap(":/image/phase_correction_green.png");
    mPhaseCorrectionRedPixmap = QPixmap(":/image/phase_correction_red.png");

    mCaptureTrackGrayPixmap = QPixmap(":/image/capture_track_gray.png");
    mCaptureTrackGreenPixmap = QPixmap(":/image/capture_track_green.png");
    mCaptureTrackRedPixmap = QPixmap(":/image/capture_track_red.png");

    mGoupGrayPixmap = QPixmap(":/image/goup_gray.png");
    mGoupGreenPixmap = QPixmap(":/image/goup_green.png");
    mGoupRedPixmap = QPixmap(":/image/goup_red.png");

    mTaskEndGrayPixmap = QPixmap(":/image/task_end_gray.png");
    mTaskEndGreenPixmap = QPixmap(":/image/task_end_green.png");
    mTaskEndRedPixmap = QPixmap(":/image/task_end_red.png");

    // 设置默认图片
    ui->taskPreparationLabel->setPixmap(mTaskPreparationGrayPixmap);
    ui->zeroCorrectionLabel->setPixmap(mZeroCorrectionGrayPixmap);
    ui->turnToWaitingPointLabel->setPixmap(mTurnToWaitingPointGrayPixmap);
    ui->phaseCorrectionLabel->setPixmap(mPhaseCorrectionGrayPixmap);
    ui->captureTrackLabel->setPixmap(mCaptureTrackGrayPixmap);
    ui->goUpLabel->setPixmap(mGoupGrayPixmap);
    ui->taskEndLabel->setPixmap(mTaskEndGrayPixmap);
}

void AutoTaskWidget::slotShowSwitchAutoRunDialog(QAbstractButton* btn)
{
    bool isAutoRun = false;
    QString text;
    if (ui->autoRunRadioBtn == btn)
    {
        text = QString("自动化运行任务");
        isAutoRun = true;
    }
    else if (ui->manualRunRadioBtn == btn)
    {
        text = QString("手动运行任务");
        isAutoRun = false;
    }
    else
    {
        return;
    }

    // 和原来的一样不进行切换
    auto oldFlag = GlobalData::getAutoRunTaskFlag();
    if (oldFlag == isAutoRun)
    {
        return;
    }

    if (QMessageBox::information(this, "提示", QString("您确定要切换为%1吗?").arg(text), QString("确定"), QString("取消")) != 0)
    {
        return;
    }

    // 设置任务运行控制类型
    GlobalData::setAutoRunTaskFlag(isAutoRun);
}

void AutoTaskWidget::slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data)
{
    // 更新图片
    slotUpdateTaskRunStepStatus(data.taskStepStatusMap);
    // 更新当前运行任务的时间信息
    slotUpdateTaskTime(data.taskPlanData.deviceWorkTask);
}

void AutoTaskWidget::slotUpdateTaskRunStepStatus(const QMap<TaskStep, TaskStepStatus>& stepStatusMap)
{
    for (TaskStep step : stepStatusMap.keys())
    {
        TaskStepStatus status = stepStatusMap[step];

        switch (step)
        {
        case TaskStep::START:  // 开始
        {
        }
        break;
        case TaskStep::TASK_PREPARATION:  // 任务准备
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->taskPreparationLabel->setPixmap(mTaskPreparationGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->taskPreparationLabel->setPixmap(mTaskPreparationGreenPixmap)
                                   : ui->taskPreparationLabel->setPixmap(mTaskPreparationGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->taskPreparationLabel->setPixmap(mTaskPreparationGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->taskPreparationLabel->setPixmap(mTaskPreparationGreenPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::ZERO_CORRECTION:  // 校零处理
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->zeroCorrectionLabel->setPixmap(mZeroCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->zeroCorrectionLabel->setPixmap(mZeroCorrectionGreenPixmap)
                                   : ui->zeroCorrectionLabel->setPixmap(mZeroCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->zeroCorrectionLabel->setPixmap(mZeroCorrectionGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->zeroCorrectionLabel->setPixmap(mZeroCorrectionRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::TASK_START:  // 任务开始
        {
        }
        break;
        case TaskStep::TURN_TOWAITINGPOINT:  // 转等待点
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->turnToWaitingPointLabel->setPixmap(mTurnToWaitingPointGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->turnToWaitingPointLabel->setPixmap(mTurnToWaitingPointGreenPixmap)
                                   : ui->turnToWaitingPointLabel->setPixmap(mTurnToWaitingPointGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->turnToWaitingPointLabel->setPixmap(mTurnToWaitingPointGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->turnToWaitingPointLabel->setPixmap(mTurnToWaitingPointRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::CALIBRATION:  // 校相处理
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->phaseCorrectionLabel->setPixmap(mPhaseCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->phaseCorrectionLabel->setPixmap(mPhaseCorrectionGreenPixmap)
                                   : ui->phaseCorrectionLabel->setPixmap(mPhaseCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->phaseCorrectionLabel->setPixmap(mPhaseCorrectionGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->phaseCorrectionLabel->setPixmap(mPhaseCorrectionRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::CAPTURE_TRACKLING:  // 捕获跟踪
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->captureTrackLabel->setPixmap(mCaptureTrackGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->captureTrackLabel->setPixmap(mCaptureTrackGreenPixmap)
                                   : ui->captureTrackLabel->setPixmap(mCaptureTrackGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->captureTrackLabel->setPixmap(mCaptureTrackGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->captureTrackLabel->setPixmap(mCaptureTrackRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::GO_UP:  // 发上行
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->goUpLabel->setPixmap(mGoupGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->goUpLabel->setPixmap(mGoupGreenPixmap) : ui->goUpLabel->setPixmap(mGoupGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->goUpLabel->setPixmap(mGoupGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->goUpLabel->setPixmap(mGoupRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::TRACK_END:        // 跟踪结束
        case TaskStep::POST_PROCESSING:  // 事后处理
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->taskEndLabel->setPixmap(mTaskEndGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                mShowRunningStatus ? ui->taskEndLabel->setPixmap(mTaskEndGreenPixmap) : ui->taskEndLabel->setPixmap(mTaskEndGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->taskEndLabel->setPixmap(mTaskEndGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->taskEndLabel->setPixmap(mTaskEndRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        }
    }

    mShowRunningStatus = !mShowRunningStatus;
}

void AutoTaskWidget::slotUpdateTaskTime(const DeviceWorkTask& deviceWorkTask)
{
    DeviceWorkTaskTarget mainTarget;
    // 有主跟目标代表任务有待执行的任务
    if (deviceWorkTask.getMainTarget(mainTarget))
    {
        ui->taskPreparationTimeLable->setText(mainTarget.m_task_ready_start_time.toString(DATETIME_DISPLAY_FORMAT2));
        ui->trackStartTimeLabel->setText(mainTarget.m_track_start_time.toString(DATETIME_DISPLAY_FORMAT2));
        ui->trackEndTimeLabel->setText(mainTarget.m_track_end_time.toString(DATETIME_DISPLAY_FORMAT2));
        ui->taskEndTimeLabel->setText(mainTarget.m_task_end_time.toString(DATETIME_DISPLAY_FORMAT2));
    }
    else
    {
        ui->taskPreparationTimeLable->setText("0000年00月00日 00:00:00");
        ui->trackStartTimeLabel->setText("0000年00月00日 00:00:00");
        ui->trackEndTimeLabel->setText("0000年00月00日 00:00:00");
        ui->taskEndTimeLabel->setText("0000年00月00日 00:00:00");
    }
}
