#ifndef TASKREPORTTABLEWIDGET_H
#define TASKREPORTTABLEWIDGET_H
#include <QWidget>

namespace Ui
{
    class TaskReportTableWidget;
}

class PageNavigator;
class TaskReportTableModel;
class TaskReportTableWidget : public QWidget
{
    Q_OBJECT

public:
    TaskReportTableWidget(QWidget* parent = nullptr);
    ~TaskReportTableWidget();

public slots:
    void currentPageChanged(int page);

signals:
    void signalGetHistoryPlan(const QByteArray&);
    void signalGetDataByUUID(const QByteArray&);

private:
    void init();

public slots:
    void slotGetHistoryPlan(const QByteArray&);
    void slotGetDataByUUID(const QByteArray&);

private slots:
    void slotExportBtnClicked();

    void slotQueryBtnClicked();

    void on_pagesize_currentIndexChanged(const QString& arg1);

    void on_taskStatusComboBox_currentIndexChanged(const QString& arg1);

    void on_taskPlanTableView_clicked(const QModelIndex& index);

private:
    Ui::TaskReportTableWidget* ui;
    TaskReportTableModel* mTaskReportTableModel = nullptr;
    PageNavigator* d_pageNavigator;
    bool m_initFinish = false;
};
#endif  // TASKREPORTTABLEWIDGET_H
