#ifndef CONFIGMACRODIALOG_H
#define CONFIGMACRODIALOG_H

#include <QDialog>

#include "ConfigMacroMessageDefine.h"
#include "ServiceCheck.h"

namespace Ui
{
    class ConfigMacroDialog;
}

class CheckBoxComboBox;
class WorkSystem;
class QTreeWidget;
class QListWidgetItem;
class QCheckBox;
class ConfigMacroDialog : public QDialog
{
    Q_OBJECT

public:
    enum WidgetMode
    {
        Add = 0,  // 添加
        Edit,     // 编辑
        Look      // 查看
    };
    explicit ConfigMacroDialog(QWidget* parent = nullptr);
    ~ConfigMacroDialog();

    // 重置数据
    void reset();

    void setWidgetMode(const WidgetMode& mode);
    WidgetMode getWidgetMode() const;

    void setConfigMacroData(const ConfigMacroData& configMacroData);

private:
    void init();

signals:
    void signalDataReady(const ConfigMacroData& configMacroData);

public slots:
    void slotOperationResult(bool result, const QString& msg);

private slots:
    // 工作模式选择改变
    void slotWorkModeSelectedChanged(QCheckBox* checkBox);
    // 确定按钮点击
    void slotOkBtnClicked();

    void on_desc_textChanged();

private:
    Ui::ConfigMacroDialog* ui;

    WidgetMode mMode = Add;  // 当前弹出框模式
    QString mTipsMessage;

    CheckBoxComboBox* mWorkModeComboBox = nullptr;
    QMap<SystemWorkMode, QTreeWidget*> mWorkModeTreeWidgetMap;  // 工作模式tab QMap<工作模式, 控件>

    ConfigMacroData mEditModeConfigMacroData;  // 编辑模式时设置的配置宏数据
};

#endif  // CONFIGMACRODIALOG_H
