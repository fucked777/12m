#ifndef SDTASKWORKMODETASKCODESELECTWIDGET_H
#define SDTASKWORKMODETASKCODESELECTWIDGET_H

#include <QButtonGroup>
#include <QWidget>

#include "PlanRunMessageDefine.h"
#include "WorkModeXMLReader.h"

namespace Ui
{
    class SDTaskWorkModeTaskCodeSelectWidget;
}

class QComboBox;
class SDTaskWorkModeTaskCodeSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SDTaskWorkModeTaskCodeSelectWidget(QWidget* parent = nullptr);
    ~SDTaskWorkModeTaskCodeSelectWidget();

    SystemWorkMode getWorkMode();
    void setWorkMode(SystemWorkMode workMode);

    QComboBox* getWorkModeComboBox();

    // 是否选择了任务代号
    bool isSelectedTaskCode();

    void setWorkModeTaskCodeMap(const QMap<SystemWorkMode, QList<QString>>& workModeTaskCodeMap);
    void setTaskCodeWorkModeDpMap(const QMap<QString, QMap<SystemWorkMode, int>>& taskCodeWorkModeDpMap);

    LinkLine getLinkLink();

    // 手动控制界面获取选择的设备信息，组成参数宏数据
    void getConfigMacroData(ConfigMacroData& configMacroData);

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

    // 特殊处理4424高速数传任务代号下拉框改变时
    void slotGSTaskCodeComboChanged();

private:
    Ui::SDTaskWorkModeTaskCodeSelectWidget* ui;

    WorkSystem mWorkSystem;

    QButtonGroup mSDTargetNoBtnGroup;                 // 手动控制界面主跟目标号按钮组
    QMap<SystemWorkMode, QWidget*> mDeviceWidgetMap;  // 手动控制栈式控件

    QMap<SystemWorkMode, QList<QString>> mWorkModeTaskcodeMap;        // 工作模式对应的任务代号的列表
    QMap<QString, QMap<SystemWorkMode, int>> mTaskCodeWorkModeDpMap;  // QMap<任务代号, QMap<工作模式, 点频数>>
};

#endif  // SDTASKWORKMODETASKCODESELECTWIDGET_H
