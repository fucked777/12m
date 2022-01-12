#ifndef DeviceWorkTaskWidget_H
#define DeviceWorkTaskWidget_H

#include "LocalPlanDialog.h"
#include "TaskPlanMessageDefine.h"
#include <QDialog>

namespace Ui
{
    class DeviceWorkTaskWidget;
}
class PlanListWidget;
class DeviceWorkTaskWidgetImpl;
class DeviceWorkTaskWidget : public QDialog
{
    Q_OBJECT
public:
    explicit DeviceWorkTaskWidget(WidgetMode widgetMode, PlanListWidget* parent);
    ~DeviceWorkTaskWidget() override;
    // 查看时设置界面数据
    void setWidgetData(const DeviceWorkTask& deviceWorkTask, const DataTranWorkTaskList& dataTranWorkTaskList);

private:
    void paintEvent(QPaintEvent* event) override;
    void resetTableBarCount(int targetCount); /* 重置当前的TableBar的数量 */
    void addACK(const QByteArray& data);
    void updateItemACK(const QByteArray& data);
    void taskTargetTotalChanged(); /* 目标总数改变 */
    void targetChange(int target); /* 当前目标改变 */
    void fullMainTarget();         /* 主跟目标数据填充 */
    void refreshCurTargetData();   /* 刷新当前目标的数据 */
    void saveCurTargetData();      /* 保存当前目标的数据 */
    void initAllTargetData(); /* 初始化的时候所有目标的数据都一样都重置为当前默认的数据,不能直接使用结构体的初始化数据 */
    void ok();

    bool checkModeRelation(const TaskPlanData& taskPlanData, QString& errorMsg);

private:
    Ui::DeviceWorkTaskWidget* ui;
    DeviceWorkTaskWidgetImpl* m_impl;
};

#endif  // DeviceWorkTaskWidget_H
