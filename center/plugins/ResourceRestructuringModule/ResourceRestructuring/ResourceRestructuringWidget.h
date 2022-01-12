#ifndef RESOURCERESTRUCTURINGWIDGET_H
#define RESOURCERESTRUCTURINGWIDGET_H

#include <QComboBox>
#include <QList>
#include <QListWidgetItem>
#include <QWidget>

#include "ResourceRestructuringMessageDefine.h"
#include <QTreeWidgetItem>
namespace Ui
{
    class ResourceRestructuringWidget;
}
namespace cppmicroservices
{
    class BundleContext;
}
class ResourceRestructuringEntity;
class ResourceRestructuringDialog;
class ResourceRestructuringTableModel;

class ResourceRestructuringWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResourceRestructuringWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~ResourceRestructuringWidget();

protected:
    void showEvent(QShowEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    void init();

signals:
    void signalAdd(const QByteArray& data);
    void signalDelete(const QByteArray& data);
    void signalQuery(const QByteArray& data);
    void signalEdit(const QByteArray& data);

    // 这是发送给弹窗的界面的
    void signalOperationDialog(bool, const QString& msg = QString());

public slots:
    void slotAddItemACK(const QByteArray& data);
    void slotDeleteItemACK(const QByteArray& data);
    void slotQueryItemACK(const QByteArray& data);
    void slotModifyItemACK(const QByteArray& data);

private slots:
    void slotAddBtnClicked();
    void slotDeleteBtnClicked();
    void slotRefreshBtnClicked();
    void slotEditBtnClicked();
    void on_resourceTableView_changed(const QModelIndex& index);
    //    void slotTableView
    void slotDialogDataReady(const ResourceRestructuringData& resourceRestructuringData);

    void on_desc_textChanged();

private:
    Ui::ResourceRestructuringWidget* ui;

    QMenu* mMenu = nullptr;

    ResourceRestructuringDialog* mDialog = nullptr;
    ResourceRestructuringTableModel* mTableModel = nullptr;

    const int ROLE = Qt::UserRole + 1;

    //    bool m_isInit = false;
    //    int m_timerId = -1;
};

#endif  // RESOURCERESTRUCTURINGWIDGET_H
