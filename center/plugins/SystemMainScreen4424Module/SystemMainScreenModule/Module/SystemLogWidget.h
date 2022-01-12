#ifndef SYSTEMLOGWIDGET_H
#define SYSTEMLOGWIDGET_H

#include <QWidget>

namespace Ui
{
    class SystemLogWidget;
}

struct SystemLogData;
class SystemLogTableModel;
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

private:
    Ui::SystemLogWidget* ui;

    QMenu* mMenu = nullptr;
    SystemLogTableModel* mSystemLogTableModel = nullptr;
};

#endif  // SYSTEMLOGWIDGET_H
