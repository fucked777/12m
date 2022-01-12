#ifndef SYSTEMLOGWIDGET_H
#define SYSTEMLOGWIDGET_H

#include <QWidget>

namespace Ui
{
    class SystemLogWidget;
}

struct SystemLogData;
class SystemLogTableFastPhotoModel;
class SystemLogTableModel;
class SystemLogFastPhotoWidget;
class QMenu;

// 系统日志模块
class SystemLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemLogWidget(QWidget* parent = nullptr);
    ~SystemLogWidget();

private:
    void init();

public slots:
    // 收到系统日志
    void slotReadSystemLog(const SystemLogData& data);
    // 表格点击
    void slotLogTableClicked(const QModelIndex& index);
    //显示快照日志界面
    void slotShowFastPhotoLogWidget();

private:
    Ui::SystemLogWidget* ui;

    QMenu* mMenu = nullptr;
    SystemLogTableModel* mSystemLogTableModel = nullptr;

    SystemLogFastPhotoWidget* m_fastWidget = nullptr;
};

#endif  // SYSTEMLOGWIDGET_H
