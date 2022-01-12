#ifndef DEVICESTATUSLOGWIDGET_H
#define DEVICESTATUSLOGWIDGET_H
#include <QWidget>

namespace Ui
{
    class DeviceStatusLogWidget;
}

struct DeviceStatusLogData;

using DeviceStatusLogDataList = QList<DeviceStatusLogData>;
class DeviceStatusLogTableModel;
class SqlDeviceStatusManager;
class PageNavigator;

class DeviceStatusLogWidget : public QWidget
{
    Q_OBJECT

public:
    DeviceStatusLogWidget(QWidget* parent = nullptr);
    ~DeviceStatusLogWidget();

private:
    void init();
    void processExport(const QString& fileName);

public slots:
    void slotSearchAck(const QByteArray& data);

private slots:
    void slotCurrentDeviceChanged();
    void slotCurrentModeChanged();
    void query();
    void exportStatus();
    void currentPageChanged(int page);
    void slotGetTotalCount(const int totalCount, const int currentPage);
    void pageSizeChange(const QString& arg1);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::DeviceStatusLogWidget* ui;
    DeviceStatusLogDataList d_dataList;
    DeviceStatusLogTableModel* mDeviceStatusLogTableModel = nullptr;
    SqlDeviceStatusManager* m_statusSqlQuery = nullptr;
    PageNavigator* d_pageNavigator;
};
#endif  // DEVICESTATUSLOGWIDGET_H
