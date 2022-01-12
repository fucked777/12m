#ifndef SCTASKWORKMODETASKCODESELECTWIDGET_H
#define SCTASKWORKMODETASKCODESELECTWIDGET_H

#include <QButtonGroup>
#include <QWidget>

#include "PlanRunMessageDefine.h"
#include "WorkModeXMLReader.h"

namespace Ui
{
    class SCTaskWorkModeTaskCodeSelectWidget;
}

class QComboBox;
class SCTaskWorkModeTaskCodeSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SCTaskWorkModeTaskCodeSelectWidget(QWidget* parent = nullptr);
    ~SCTaskWorkModeTaskCodeSelectWidget();

    SystemWorkMode getWorkMode();
    void setWorkMode(SystemWorkMode workMode);

    // 是否选择了任务代号
    bool isSelectedTaskCode();

    void setWorkModeTaskCodeMap(const QMap<SystemWorkMode, QList<QString>>& workModeTaskCodeMap);
    void setTaskCodeWorkModeDpMap(const QMap<QString, QMap<SystemWorkMode, int>>& taskCodeWorkModeDpMap);

    LinkLine getLinkLine();

    void refreshRunningTaskPlanData(const ManualMessage& msg);
    void refreshMasterTargetNo(int checkID);
    void refreshTargetInfo(const QMap<int, TargetInfo>& targetMap);

private:
    void initUI();
    void initSlot();

    // 根据任务代号，改变对于的点频
    void changedPointFreq(QComboBox* taskCodeCombo, QComboBox* pointFreqCombo);

signals:
    void signalWorkModeChanged(SystemWorkMode workMode);

private slots:
    // 模式改变
    void slotWorkModeChanged(int index);
    // 根据工作模式，改变当前工作模式能够选用的目标个数
    void slotChangeTarget(SystemWorkMode workMode);

private:
    Ui::SCTaskWorkModeTaskCodeSelectWidget* ui;

    WorkSystem mWorkSystem;

    SystemWorkMode mdefaultSystemWorkMode;

    QButtonGroup mSCTargetNoBtnGroup;  // 快捷控制界面数传主跟目标号按钮组

    QMap<SystemWorkMode, QList<QString>> mWorkModeTaskcodeMap;        // 工作模式对应的任务代号的列表
    QMap<QString, QMap<SystemWorkMode, int>> mTaskCodeWorkModeDpMap;  // QMap<任务代号, QMap<工作模式, 点频数>>
};

#endif  // SCTASKWORKMODETASKCODESELECTWIDGET_H
