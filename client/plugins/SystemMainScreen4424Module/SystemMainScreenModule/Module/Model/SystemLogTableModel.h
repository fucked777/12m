#ifndef SYSTEMLOGTABLEMODEL_H
#define SYSTEMLOGTABLEMODEL_H

#include <QAbstractTableModel>

struct SystemLogData;
class SystemLogTableModel : public QAbstractTableModel
{
    Q_OBJECT

    enum Column
    {
        Type,    // 日志类型
        Time,    // 时间
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
    void prependSystemLogData(const SystemLogData& systemLogData);
    void clear();

private:
    QStringList mHeaders;
    QList<SystemLogData> mSystemLogDatas;
};

#endif  // SYSTEMLOGTABLEMODEL_H
