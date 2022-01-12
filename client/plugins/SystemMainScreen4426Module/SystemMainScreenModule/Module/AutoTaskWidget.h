#ifndef AUTOTASKWIDGET_H
#define AUTOTASKWIDGET_H

#include <QWidget>

#include "QButtonGroup"
#include "TaskPlanMessageDefine.h"

namespace Ui
{
    class AutoTaskWidget;
}

// 自动任务模块
class AutoTaskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutoTaskWidget(QWidget* parent = nullptr);
    ~AutoTaskWidget();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    // 初始化
    void init();
    // 初始化图片
    void initPixmap();

private slots:
    // 自动或者手动运行按钮点击
    void slotShowSwitchAutoRunDialog(QAbstractButton* btn);
    // 读取任务运行流程数据
    void slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data);
    // 更新任务运行流程的步骤状态
    void slotUpdateTaskRunStepStatus(const QMap<TaskStep, TaskStepStatus>& stepStatusMap);
    // 更新当前任务时间信息
    void slotUpdateTaskTime(const DeviceWorkTask& deviceWorkTask);

private:
    Ui::AutoTaskWidget* ui;

    bool mShowRunningStatus = false;

    QButtonGroup mBtnGroup;

    QPixmap mTaskPreparationGrayPixmap;
    QPixmap mTaskPreparationGreenPixmap;
    QPixmap mTaskPreparationRedPixmap;

    QPixmap mZeroCorrectionGrayPixmap;
    QPixmap mZeroCorrectionGreenPixmap;
    QPixmap mZeroCorrectionRedPixmap;

    QPixmap mTurnToWaitingPointGrayPixmap;
    QPixmap mTurnToWaitingPointGreenPixmap;
    QPixmap mTurnToWaitingPointRedPixmap;

    QPixmap mPhaseCorrectionGrayPixmap;
    QPixmap mPhaseCorrectionGreenPixmap;
    QPixmap mPhaseCorrectionRedPixmap;

    QPixmap mCaptureTrackGrayPixmap;
    QPixmap mCaptureTrackGreenPixmap;
    QPixmap mCaptureTrackRedPixmap;

    QPixmap mGoupGrayPixmap;
    QPixmap mGoupGreenPixmap;
    QPixmap mGoupRedPixmap;

    QPixmap mTaskEndGrayPixmap;
    QPixmap mTaskEndGreenPixmap;
    QPixmap mTaskEndRedPixmap;
};

#endif  // AUTOTASKWIDGET_H
