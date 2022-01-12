#ifndef CREATETESTPLANWIDGET_H
#define CREATETESTPLANWIDGET_H

#include <QDialog>

namespace Ui
{
    class CreateTestPlanWidget;
}
enum class FrequencyBand;
enum class SCWorkMode;
class XmlNodeData;
class QTreeWidgetItem;
class SystemCalibrationItem;
class IGlobalData;
class UIXmlConfigure;
class CreateTestPlanWidgetImpl;
struct AutoTestInfo;

struct AutomateTestItemInfo;
class AutomateTestWidget;
class CreateTestPlanWidget : public QDialog
{
    Q_OBJECT
public:
    explicit CreateTestPlanWidget(AutomateTestWidget* parent);
    ~CreateTestPlanWidget() override;

    void init(const AutomateTestItemInfo& testItemInfo);
    void addTestItemACK(const QByteArray& data);

private:
    void selectTestItem(QTreeWidgetItem* item, int column); /* 选择了一个测试项 */
    void addTestItem();

private:
    void showEvent(QShowEvent*) override;
    void reloadUIData();                                             /* 重载ui数据 */
    void taskCodeChanged();                                          /* 任务代号切换 */
    void workModeChanged();                                          /* 工作模式切换 */
    void setNotFoundParam(const QString& tips = "未查找到测试参数"); /* 参数未找到 */
    void clearAllItem();                                             /* 清除所有的选中项目 */

signals:
    // void sg_fbChange(FrequencyBand);
    // void sg_wbChange(SCWorkMode);

    // void signal_sendCmdAutoTest(const QByteArray& data);

public slots:
    // void slotSendCmdAutoTestACK(const QByteArray& data);

private:
    Ui::CreateTestPlanWidget* ui;
    CreateTestPlanWidgetImpl* m_impl;
};

#endif  // CREATETESTPLANWIDGET_H
