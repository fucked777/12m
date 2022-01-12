#ifndef CKTASKWORKMODETASKCODESELECTWIDGET_H
#define CKTASKWORKMODETASKCODESELECTWIDGET_H

#include <QButtonGroup>
#include <QWidget>

#include "PlanRunMessageDefine.h"
#include "WorkModeXMLReader.h"

namespace Ui
{
    class CKTaskWorkModeTaskCodeSelectWidget;
}
class QComboBox;
class CKTaskWorkModeTaskCodeSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CKTaskWorkModeTaskCodeSelectWidget(QWidget* parent = nullptr);
    ~CKTaskWorkModeTaskCodeSelectWidget();

    QComboBox* getWorkModeComboBox();

    SystemWorkMode getWorkMode();
    void setWorkMode(SystemWorkMode workMode);

    // 是否选择了任务代号
    bool isSelectedTaskCode();

    void setWorkModeTaskCodeMap(const QMap<SystemWorkMode, QList<QString>>& workModeTaskCodeMap);
    void setTaskCodeWorkModeDpMap(const QMap<QString, QMap<SystemWorkMode, int>>& taskCodeWorkModeDpMap);

    LinkLine getLinkLine();

    void setWorkModeType(int workModeType);
    void initUI();

private:
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
    Ui::CKTaskWorkModeTaskCodeSelectWidget* ui;

    WorkSystem mWorkSystem;

    QButtonGroup mCkTargetNoBtnGroup;  // 快捷控制界面测控第一个Tab主跟目标号按钮组

    QMap<SystemWorkMode, QList<QString>> mWorkModeTaskcodeMap;        // 工作模式对应的任务代号的列表
    QMap<QString, QMap<SystemWorkMode, int>> mTaskCodeWorkModeDpMap;  // QMap<任务代号, QMap<工作模式, 点频数>>

    int m_workModeType = 0;  // 0代表只有S，1代表只有Ka，为了满足憨憨需求
};

#endif  // CKTASKWORKMODETASKCODESELECTWIDGET_H
