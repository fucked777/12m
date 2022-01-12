#ifndef TASKPROCESSCONTROLWIDGET2_H
#define TASKPROCESSCONTROLWIDGET2_H

#include <QButtonGroup>
#include <QWidget>

#include "CCZFTaskPlanWidget.h"
#include "PlanRunMessageDefine.h"

namespace Ui
{
    class TaskProcessControlWidget;
}

struct SystemLogData;
class HP;
class SystemCaptureProcessControl;
struct StatusBarData;
class StatusBarManagerThread;
class TaskProcessControlWidget : public QWidget
{
    Q_OBJECT

    enum UiLinkType
    {
        CKLink,
        DTLink,
        CK_DTLink
    };

public:
    explicit TaskProcessControlWidget(QWidget* parent = nullptr);
    ~TaskProcessControlWidget();

protected:
    void showEvent(QShowEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void initUI();
    void initSlot();

    // 去改变系统工作方式
    void changeSystemWorkWay();

    // 功放显示策略
    void hpTableReare();

    //链路控制的方法
    void manualFunction(ManualType type, LinkType linkType, UiLinkType uilinkType);
    void manualXLFunction(bool sOrKa); /* true S false Ka */
    //链路控制的方法
    void manualDtFunction(ManualType type, LinkType linkType, UiLinkType uilinkType, int _type);

    // 这里是判断界面没有选择工作模式或没有选择任务代号的提示
    bool checkMistake();
    bool checkXLMistake(bool sOrKa); /* true S false Ka */

    //处理主跟目标代号
    void dealMainTrackTargetInfo(ManualMessage& manualMsg);

    //点击参数宏按钮后重置界面部分参数信息
    void resetUIData();

    void refreshRunningTaskPlanData();  //每次打开界面时刷新一次当前任务信息

public slots:
    void slotCmdResponse(const QByteArray& data);

signals:
    void signalManualFunction(const QString& json);
    void signalsCmdDeviceJson(const QString& json);
    void signalsUnitDeviceJson(const QString& json);

private slots:
    // 刷新卫星数据
    void slotRefreshData();

    // 工作模式改变
    void slotWorkModeChanged(SystemWorkMode workMode);
    // 系统工作方式改变
    void slotSystemWorkWayChanged();

    // 当测控、数传模式改变，或者系统工作方式改变时，是否显示扩频和数传单选按钮
    void slotIsShowRadioBtn();

    // 资源释放按钮点击
    void slotResourceReleaseBtnClick();
    // 参数宏下发按钮点击
    void slotSetParamMacroBtnClick();
    // 配置宏设置按钮点击
    void slotLinkSetBtnClick();
    // 一键校零按钮点击
    void slotSOneKeyXLBtnClick();
    void slotKaOneKeyXLBtnClick();
    // 收到系统日志
    void slotReadSystemLog(const SystemLogData& data);
    //测控设置按钮
    void slotsCkSettingBtnClick();
    //数传设置按钮
    void slotsDtSettingBtnClick();

    //刷新离线分控设备
    void slotRefreshUI(const StatusBarData& statusBarData);
    // 读取任务运行流程数据
    void slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data);

private:
    Ui::TaskProcessControlWidget* ui;

    int m_timerID{ -1 };

    SystemCaptureProcessControl* m_systemCaptureProcessControl = nullptr;

    /* 高功放要用tab页面 有3个 S ka测控 ka数传 */
    HP* m_sHP;
    HP* m_kaSCHP;
    HP* m_kaCKHP;

    QButtonGroup mMasterSlaveLinkBtnGroup;  // 主备链路按钮组

    ManualMessage m_runningTaskPlanDataMsg;

    //所有卫星信息
    QMap<QString, SatelliteManagementData> m_taskCodeSateliteMap;

    StatusBarManagerThread* m_statusBarThread = nullptr;
    QMap<DeviceID, QString> m_offDeviceNameMap;  //离线设备名字
    QStringList m_offDeviceNameList;             //离线设备名字List
    QMap<DeviceID, QString> m_subDeviceNameMap;  //分控设备名字
    QStringList m_subDeviceNameList;             //分控设备名字List
};

#endif  // TASKPROCESSCONTROLWIDGET2_H
