#ifndef TASKEXECUTELOGWIDGET_H
#define TASKEXECUTELOGWIDGET_H
#include <QWidget>

namespace Ui
{
    class TaskExecuteLogWidget;
}

class PageNavigator;
class TaskPlanTableModel;
class AutoTaskLogTableModel;
class TaskExecuteLogWidget : public QWidget
{
    Q_OBJECT

public:
    TaskExecuteLogWidget(QWidget* parent = nullptr);
    ~TaskExecuteLogWidget();

public slots:
    void currentPageChanged(int page);

signals:
    void signalGetHistoryPlan(const QByteArray&);
    void signalQueryAutoTaskLog(const QByteArray&);

private:
    void init();

public slots:
    void slotGetHistoryPlan(const QByteArray&);
    void slotQueryAutoTaskLog(const QByteArray&);

private slots:
    void slotQueryBtnClicked();

    void on_pagesize_currentIndexChanged(const QString& arg1);

    void on_taskStatusComboBox_currentIndexChanged(const QString& arg1);

    void on_taskPlanTableView_clicked(const QModelIndex& index);

private:
    Ui::TaskExecuteLogWidget* ui;

    TaskPlanTableModel* mTaskPlanTableModel = nullptr;
    AutoTaskLogTableModel* mAutoTaskLogTableModel = nullptr;
    PageNavigator* d_pageNavigator;
    bool m_initFinish = false;
};

#endif  // TASKEXECUTELOGWIDGET_H
