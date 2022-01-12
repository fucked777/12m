#ifndef AUTOTASKLOGTABLEMODEL_H
#define AUTOTASKLOGTABLEMODEL_H

#include "Utility.h"
#include <QAbstractTableModel>

struct AutoTaskLogData;
class AutoTaskLogTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        CreateTime,  // 创建时间
        TaskCode,    // 任务代号
        Level,       // 级别
        Context,     // 内容
    };

    explicit AutoTaskLogTableModel(QObject* parent = nullptr);
    ~AutoTaskLogTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setAutoTaskLogDatas(const QList<AutoTaskLogData>& list);
    Optional<AutoTaskLogData> getAutoTaskLogData(const QModelIndex& index) const;

private:
    QStringList mHeaders;
    QList<AutoTaskLogData> mAutoTaskLogs;
};

#endif  // AUTOTASKLOGTABLEMODEL_H
