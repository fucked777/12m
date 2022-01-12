#ifndef LOCALPLANDIALOG_H
#define LOCALPLANDIALOG_H

#include "TaskPlanMessageDefine.h"
#include "Utility.h"
#include <QWidget>
class XComboBox;
namespace Ui
{
    class LocalPlanDialog;
}

// 新建计划、编辑计划、查看计划
enum WidgetMode
{
    New = 0,  // 新建
    View,     // 查看
    Edit,     // 编辑
};

class DataCenterData;
struct QStatePushButtonInfo;
class LocalPlanDialogImpl;

struct DataTransmissionCenterData;
using DataTransmissionCenterMap = QMap<QString, DataTransmissionCenterData>;

struct SatelliteManagementData;
using SatelliteManagementDataMap = QMap<QString, SatelliteManagementData>;

class LocalPlanDialog : public QWidget
{
    Q_OBJECT

public:
    explicit LocalPlanDialog(QWidget* parent = nullptr);
    ~LocalPlanDialog() override;
    void setSatelliteMap(const SatelliteManagementDataMap& satelliteMap);
    void setSCCenterData(const DataTransmissionCenterMap&);
    void setWidgetMode(WidgetMode widgetMode);

    //编辑时让控件不可编辑
    void setControlEnable();

    // void setData();
    bool dtEffective();

    Optional<DeviceWorkTaskTarget> getDeviceWorkTaskTarget();
    void setDeviceWorkTaskTarget(const DeviceWorkTaskTarget& deviceWorkTarget);

    Optional<DataTranWorkTask> getDataTranWorkTask();
    void setDataTranWorkTask(const DataTranWorkTask& dataTranWorkTask);

    void setCurTarget(int target);
    int curTarget();
    QString curTaskCode();

    bool checkRelation(QString& errmsg);

    void refreshDefaultWorkMode(const SatelliteManagementData& info);

signals:
    /* 任务代号切换时,要切换主跟目标的选择 */
    void signalCurTargetTaskCodeChanged();

private slots:

    void on_workmode_cbx1_currentIndexChanged(int index);
    void on_workmode_cbx2_currentIndexChanged(int index);
    void on_workmode_cbx3_currentIndexChanged(int index);
    void on_workmode_cbx4_currentIndexChanged(int index);
    void on_workmode_cbx5_currentIndexChanged(int index);
    void on_workmode_cbx6_currentIndexChanged(int index);
    void on_workmode_cbx7_currentIndexChanged(int index);
    void on_workmode_cbx8_currentIndexChanged(int index);

    void on_pointfrequency_cbx1_currentIndexChanged(int index);
    void on_pointfrequency_cbx2_currentIndexChanged(int index);
    void on_pointfrequency_cbx3_currentIndexChanged(int index);
    void on_pointfrequency_cbx4_currentIndexChanged(int index);
    void on_pointfrequency_cbx5_currentIndexChanged(int index);
    void on_pointfrequency_cbx6_currentIndexChanged(int index);
    void on_pointfrequency_cbx7_currentIndexChanged(int index);
    void on_pointfrequency_cbx8_currentIndexChanged(int index);

private:
    void taskTypeChange(int);

    /* 切换全零全F的时候切换启用或者不启用 */
    void openRemoteControlChanged(int id);
    void openUpSignalChanged(int id);
    /* 任务代号改变 */
    void targetTaskCodeChanged(const QString& arg1);

private:
    Ui::LocalPlanDialog* ui;
    LocalPlanDialogImpl* m_impl;
    XComboBox* m_target_center = nullptr;  //数传中心下拉复选框
};

#endif  // LOCALPLANDIALOG_H
