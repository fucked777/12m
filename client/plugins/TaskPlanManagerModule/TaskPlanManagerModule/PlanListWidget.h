#ifndef PLANLISTWIDGET_H
#define PLANLISTWIDGET_H

#include <QButtonGroup>
#include <QWidget>

#include "RedisHelper.h"
#include "TaskPlanMessageDefine.h"

namespace Ui
{
    class PlanListWidget;
}
class PlanListWidgetPrivate;
class DeviceWorkTaskWidget;
class DataTransferPlanWidget;
class RealtimeLogModel;
struct AutoTaskLogData;
class PlanListWidgetImpl;
class PlanListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlanListWidget(QWidget* parent = nullptr);
    ~PlanListWidget() override;

    void deletePlanItemACK(const QByteArray&);
    void viewItemACK(const QByteArray&);
    void editInfoItemACK(const QByteArray&);
    // void queryDeviceItemACK(const QByteArray&);
    // void queryDTItemACK(const QByteArray&);

signals:
    void sg_addItem(const QByteArray&);
    void sg_deletePlanItem(const QByteArray&);
    void sg_viewItem(const QByteArray&);
    void sg_editInfoItem(const QByteArray&);
    void sg_updateItem(const QByteArray&);
    // void sg_queryDeviceItem(const QByteArray&);
    // void sg_queryDTItem(const QByteArray&);
    void sg_addItemACK(const QByteArray&);
    void sg_updateItemACK(const QByteArray&);

private slots:
    // 读取任务运行流程数据
    void slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data);
    // 读取自动运行任务日志
    void slotReadAutoRunTaskLog(const AutoTaskLogData& data);

    // 更新任务运行流程的步骤状态
    void slotUpdateTaskRunStepStatus(const QMap<TaskStep, TaskStepStatus>& stepStatusMap);
    // 更新当前任务时间信息
    void slotUpdateTaskTime(const DeviceWorkTask& deviceWorkTask);
    // 自动或者手动运行按钮点击
    void slotShowSwitchAutoRunDialog(QAbstractButton* btn);

    //显示快照日志界面
    void slotShowFastPhotoLogWidget();

private:
    void autoRefreshChange(bool clickAuto);                           /* 在定时刷新和手动刷新之间切换 */
    void autoRefreshData();                                           /* 自动判断是自动刷新还是手动刷新,自动刷新数据 */
    void timerOutRefresh();                                           /* 定时刷新,只刷新未运行和正在运行的 */
    void manualRefreshData(const QDate& startDt, const QDate& endDt); /* 手动刷新 */
    void startTimerOutRefresh();                                      /* 定时刷新 开 */
    void stopTimerOutRefresh();                                       /* 定时刷新 关 */
    bool enableTimerOutRefresh() const;                               /* 当前是否是定时刷新 */

    void addItem();                                          /* 按钮点击  添加 */
    void deleteItem();                                       /* 按钮点击  删除 */
    void viewItem();                                         /* 按钮点击  查看 */
    void editItem();                                         /* 按钮点击  编辑 */
    void getPlan();                                          /* 按钮点击  获取计划 */
    void queryByTime();                                      /* 按钮点击  指定时间查询 */
    void projectTableSelectChange(const QModelIndex& index); /* 点击列表 */
    void dtTableSelectChange(const QModelIndex& index);      /* 点击列表 */

    void initUI();
    void initPixmap();
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    Ui::PlanListWidget* ui;
    PlanListWidgetImpl* m_impl;

    QButtonGroup m_btnGroup;
};

#endif  // PLANLISTWIDGET_H
