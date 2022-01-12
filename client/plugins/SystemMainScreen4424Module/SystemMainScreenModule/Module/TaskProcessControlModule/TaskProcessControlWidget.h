#ifndef TASKPROCESSCONTROLWIDGET2_H
#define TASKPROCESSCONTROLWIDGET2_H

#include <QButtonGroup>
#include <QWidget>

#include "PlanRunMessageDefine.h"

namespace Ui
{
    class TaskProcessControlWidget;
}

struct SystemLogData;
class HP;
class SystemCaptureProcessControl;

class TaskProcessControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskProcessControlWidget(QWidget* parent = nullptr);
    ~TaskProcessControlWidget();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void initUI();
    void initSlot();

    // 去改变系统工作方式
    void changeSystemWorkWay();

    // 功放显示策略
    void hpTableReare();

    //链路控制的方法
    void manualFunction(ManualType type);
    // 这里是判断界面没有选择工作模式或没有选择任务代号的提示
    bool checkMistake();

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

    // 快捷控制和手动控制tab界面切换
    void slotContrlTabChanged(int index);
    // 测控tab界面切换
    void slotCKTabChanged(int index);

    // 资源释放按钮点击
    void slotResourceReleaseBtnClick();
    // 参数宏下发按钮点击
    void slotSetParamMacroBtnClick();
    // 链路设置按钮点击
    void slotLinkSetBtnClick();
    // 一键校零按钮点击
    void slotOneKeyXLBtnClick();
    // 收到系统日志
    void slotReadSystemLog(const SystemLogData& data);

private:
    Ui::TaskProcessControlWidget* ui;

    SystemCaptureProcessControl* m_systemCaptureProcessControl = nullptr;

    /* 高功放要用tab页面 有3个 S ka测控 ka数传 */
    HP* m_sHP;
    HP* m_kaSCHP;
    HP* m_kaCKHP;

    QButtonGroup mMasterSlaveLinkBtnGroup;  // 主备链路按钮组
};

#endif  // TASKPROCESSCONTROLWIDGET2_H
