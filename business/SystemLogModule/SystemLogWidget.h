#ifndef SYSTEMLOGWIDGET_H
#define SYSTEMLOGWIDGET_H

#include <QWidget>

namespace Ui
{
    class SystemLogWidget;
}

class SystemLogTableModel;
class PageNavigator;
class SystemLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemLogWidget(QWidget* parent = nullptr);
    ~SystemLogWidget();

signals:
    void signalLogConditionQuery(const QByteArray&);

public slots:
    void slotLogConditionQuery(const QByteArray&);

private slots:
    void logLevelChange(const QString& arg1);
    void pageSizeChange(const QString& arg1);

private:
    void init();
    void queryLogs();
    void currentPageChanged(int page);

private slots:
    //如果需要导出大量系统日志,那么应该这么做：1.把时间区间尽可能选择精准2.把分页大小设置为合适，最大为100条记录
    void slotExportBtnClicked();

    void slotQueryBtnClicked();

private:
    Ui::SystemLogWidget* ui;

    SystemLogTableModel* mSystemLogTableModel = nullptr;

    PageNavigator* d_pageNavigator;
};

#endif  // SYSTEMLOGWIDGET_H
