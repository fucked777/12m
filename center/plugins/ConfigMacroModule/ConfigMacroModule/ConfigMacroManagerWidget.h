#ifndef CONFIGMACROMANAGERWIDGET_H
#define CONFIGMACROMANAGERWIDGET_H

#include <QWidget>

namespace Ui
{
    class ConfigMacroManagerWidget;
}

class QMenu;
class ConfigMacroDialog;
class ConfigMacroTableModel;
struct ConfigMacroData;
class ConfigMacroManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigMacroManagerWidget(QWidget* parent = nullptr);
    ~ConfigMacroManagerWidget();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void init();
    // 检查服务器状态、用户权限
    bool check();

signals:
    void signalAddConfigMacro(const QByteArray& data);
    void signalDeleteConfigMacro(const QByteArray& data);
    void signalEditConfigMacro(const QByteArray& data);

    // 这是发送给弹窗的界面的
    void signalOperationDialog(bool, const QString& msg = QString());

public slots:
    void slotAddItemACK(const QByteArray& data);
    void slotDeleteItemACK(const QByteArray& data);
    void slotModifyItemACK(const QByteArray& data);

private slots:
    void slotAddBtnClicked();
    void slotDeleteBtnClicked();
    void slotRefreshBtnClicked();
    void slotEditBtnClicked();

    void slotConfigMacroDialogDataReady(const ConfigMacroData& ConfigMacroData);

private:
    Ui::ConfigMacroManagerWidget* ui;

    QMenu* mMenu = nullptr;

    ConfigMacroDialog* mConfigMacroDialog = nullptr;
    ConfigMacroTableModel* mConfigMacroTableModel = nullptr;
};

#endif  // CONFIGMACROMANAGERWIDGET_H
