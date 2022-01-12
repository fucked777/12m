#ifndef DEVICESTATUSLOGTABLEMODEL_H
#define DEVICESTATUSLOGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QThread>
#include <QSet>

class SqlDeviceStatusManager;
struct DeviceStatusLogData;
using DeviceStatusLogDataList = QList<DeviceStatusLogData>;
class DeviceStatusLogTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Cloumn
    {
        Time,       // 时间
        Device,     // 设备
        ModeId,     // 模式
        Unit,       // 单元
        Param,      // 参数
        ParamValue  // 参数值
    };

    explicit DeviceStatusLogTableModel(QObject* parent = nullptr);
    ~DeviceStatusLogTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setDeviceStatusLogData(const QList<DeviceStatusLogData>& systemLogDatas);
    QList<DeviceStatusLogData> getDeviceStatusLogData() const;

    void appendDeviceStatusLogData(const DeviceStatusLogData& systemLogData);

    void reset(const QSet<int>& devices, const QSet<QString>& units, const QDateTime& start_time, const QDateTime& end_time,
               const QSet<int>& modes = QSet<int>(), int currentPage = -1, int pageSize = -1);

    //    void resetLog(const QString device, const QString deviceType, const QDateTime& start_time, const QDateTime& end_time);

signals:
    void load(int n);
    void loaded(int n);

    void searchLog(const QSet<int>& devices, const QSet<QString>& units, const QDateTime& start_time,
                   const QDateTime& end_time, const QSet<int>& modes, int currentPage, int pageSize);

    void totalCount(const QSet<int>& devices, const QSet<QString>& units, const QDateTime& start_time,
                    const QDateTime& end_time, const QSet<int>& modes,  int currentPage);
    void signalTotalCount(const int totalCount, const int currentPage);
    //    void logsearch(const QString device, const QString deviceType, const QDateTime start_time, const QDateTime end_time);

public slots:
    //    void moreStatus(const DeviceStatusLogDataList& status);
    void searchLogAck(const DeviceStatusLogDataList& status);
    void totalCountAck(const int totalCount, const int currentPage);

protected:
    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore(const QModelIndex& parent) override;

private:
    QStringList mHeaders;

    DeviceStatusLogDataList status_list_;
    DeviceStatusLogDataList more_status_;

    SqlDeviceStatusManager* status_query_;
    QThread status_thread_;
};

#endif  // DEVICESTATUSLOGTABLEMODEL_H
