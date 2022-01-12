#ifndef REALTIMELOGMODEL_H
#define REALTIMELOGMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "AutoRunTaskLogMessageDefine.h"
class RealtimeLogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        Time,     // 日志生成时间
        TaskId,   // 任务代号
        Level,    // 日志类型
        Context,  // 日志具体内容
        Columnsize
    };
    explicit RealtimeLogModel(QObject* parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void prependLogData(const AutoTaskLogData& log);
    void setSystemLogData(const QList<AutoTaskLogData>& systemLogDatas);

    bool judgeData(int row);
    void clear();

    QList<AutoTaskLogData> getCurrentLogData();

private:
    QList<AutoTaskLogData> mAutoTaskLogData;
};

#endif  // REALTIMELOGMODEL_H
