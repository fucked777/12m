#ifndef RESOURCERESTRUCTURINGTABLEMODEL_H
#define RESOURCERESTRUCTURINGTABLEMODEL_H

#include <QAbstractTableModel>

#include "Utility.h"
#include "ResourceRestructuringMessageDefine.h"

class ResourceRestructuringTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        Name = 0,     // 名称
        WorkMode,     // 工作模式
        IsMaster,     // 主备
        Desc          // 描述
    };

    explicit ResourceRestructuringTableModel(QObject* parent = nullptr);
    ~ResourceRestructuringTableModel(){}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setDatas(const ResourceRestructuringDataList& list);
    void addData(const ResourceRestructuringData& data);
    void deleteData(const QModelIndex& index);
    void deleteData(const ResourceRestructuringData& data);
    void updateData(const ResourceRestructuringData& data);
    Optional<ResourceRestructuringData> getData(const QModelIndex& index) const;

private:
    QStringList mHeaders;
    ResourceRestructuringDataList mResourceRestructuringDatas;
};

#endif // RESOURCERESTRUCTURINGTABLEMODEL_H
