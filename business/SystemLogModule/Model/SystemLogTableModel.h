#ifndef SYSTEMLOGTABLEMODEL_H
#define SYSTEMLOGTABLEMODEL_H

#include <QAbstractTableModel>

struct SystemLogData;
class SystemLogTableModel : public QAbstractTableModel
{
    Q_OBJECT

    enum Column
    {
        //        Id,      // 系统日志唯一ID
        Time,    // 时间
        Level,   // 日志等级
        Type,    // 日志类型
        Module,  // 日志产生模块
        UserId,  // 当前用户操作ID
        Context  // 内容
    };

public:
    explicit SystemLogTableModel(QObject* parent = nullptr);
    ~SystemLogTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setSystemLogData(const QList<SystemLogData>& systemLogDatas);
    QList<SystemLogData> getSystemLogData() const;

    void appendSystemLogData(const SystemLogData& systemLogData);

private:
    QStringList mHeaders;
    QList<SystemLogData> mSystemLogDatas;
};

#endif  // SYSTEMLOGTABLEMODEL_H
