#ifndef ResourceRestructuringDialog_H
#define ResourceRestructuringDialog_H

#include "ResourceRestructuringMessageDefine.h"
#include <QComboBox>
#include <QDialog>
#include <QList>
#include <QListWidgetItem>
#include <QMap>
#include <QTreeWidgetItem>
#include <QWidget>

namespace Ui
{
    class ResourceRestructuringDialog;
}

class ResourceRestructuringDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode
    {
        Add = 0,  // 添加
        Edit,     // 编辑
    };

    explicit ResourceRestructuringDialog(QWidget* parent = 0);
    ~ResourceRestructuringDialog();

    // 重置数据
    void reset();

    void setMode(const Mode& mode);
    Mode getMode() const;

    void setData(const ResourceRestructuringData& resourceRestructuringData);
    void setMainData(const ResourceRestructuringData& resourceRestructuringData);
    void setSubData(const ResourceRestructuringData& resourceRestructuringData);

    QTreeWidget* getTreeWidget(SystemWorkMode workMode);

    QString getWorkModeName(SystemWorkMode workMode);

    QMap<SystemWorkMode, QString> getWorkModeNameMap();

private:
    void init();
    int getComBoxIndex(QComboBox* comBox, const int target);

signals:
    void signalDataReady(const ResourceRestructuringData& configMacroData);

public slots:
    void slotOperationResult(bool result, const QString& msg);

private slots:
    // 工作模式选择改变
    void slotWorkModeSelectedChanged(const QString& text);
    // 确定按钮点击
    void slotOkBtnClicked();

    void on_descs_textChanged();

private:
    Ui::ResourceRestructuringDialog* ui;

    Mode mMode = Add;  // 当前弹出框模式
    QString mTipsMessage;

    QMap<SystemWorkMode, QTreeWidget*> mWorkModeTreeWidgetMap;  // 工作模式tab QMap<模式名称, 控件>
    QMap<SystemWorkMode, QString> mWorkModeNameMap;
    QMap<int, QString> d_BBE_ID_Type;

    ResourceRestructuringData mEditResourceRestructuringData;  // 编辑模式时设置的配置宏数据
};

#endif  // ResourceRestructuringDialog_H
