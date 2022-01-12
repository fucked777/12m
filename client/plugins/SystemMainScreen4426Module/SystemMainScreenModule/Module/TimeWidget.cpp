#include "TimeWidget.h"
#include "ui_TimeWidget.h"

#include <QDateTime>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

#include "GlobalData.h"
#include "MessagePublish.h"
#include "SubscriberHelper.h"
#include "TaskPlanMessageSerialize.h"

TimeWidget::TimeWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TimeWidget)
{
    ui->setupUi(this);
    qRegisterMetaType<DeviceWorkTask>("const DeviceWorkTask&");

    setStyleSheet("QLabel {"
                  "color: rgb(9, 0, 255);"
                  "font: 75 22pt 方正楷体;"
                  "}");
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData");
    qRegisterMetaType<CurrentRunningTaskPlanData>("const CurrentRunningTaskPlanData&");
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData&");

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskStepStatusReadable, this,
            &TimeWidget::slotReadAutoRunTaskStepStatus);

    startTimer(500);
}

TimeWidget::~TimeWidget() { delete ui; }

void TimeWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)

    // 根据使用的时间类型获取当前时间
    ui->systemTimeLabel->setText(GlobalData::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void TimeWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void TimeWidget::slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data)
{
    // 更新当前运行任务的时间信息
    slotUpdateTaskTime(data.taskPlanData.deviceWorkTask);
}

void TimeWidget::slotUpdateTaskTime(const DeviceWorkTask& deviceWorkTask)
{
    DeviceWorkTaskTarget mainTarget;
    // 有主跟目标代表任务有待执行的任务
    if (deviceWorkTask.getMainTarget(mainTarget))
    {
        QDateTime currentDateTime = GlobalData::currentDateTime();
        QString countDownText;
        // 该任务已经在执行，倒计时完成
        if (currentDateTime > mainTarget.m_task_ready_start_time)
        {
            countDownText = QString("00:00:00");
        }
        else
        {
            auto secs = currentDateTime.secsTo(mainTarget.m_task_ready_start_time);

            int mi = 60;
            int hh = mi * 60;
            int dd = hh * 24;
            long day = secs / dd;
            long hour = (secs - day * dd) / hh;
            long minute = (secs - day * dd - hour * hh) / mi;
            long secend = (secs - day * dd - hour * hh - minute * mi);

            if (day > 0)
            {
                countDownText = QString("%0天%1:%2:%3")
                                    .arg(day, 2, 10, QLatin1Char('0'))
                                    .arg(hour, 2, 10, QLatin1Char('0'))
                                    .arg(minute, 2, 10, QLatin1Char('0'))
                                    .arg(secend, 2, 10, QLatin1Char('0'));
            }
            else
            {
                countDownText =
                    QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0')).arg(secend, 2, 10, QLatin1Char('0'));
            }
        }

        ui->taskCountDownLabel->setText(countDownText);
    }
    else
    {
        ui->taskCountDownLabel->setText("--:--:--");
    }
    // 强制刷新
    ui->taskCountDownLabel->repaint();
}
