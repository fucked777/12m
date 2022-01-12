#ifndef DMITEMDATAMODEL_H
#define DMITEMDATAMODEL_H

#include "Utility.h"
#include <QAbstractListModel>
#include <QJsonObject>

struct DynamicWidgetInfo;
struct DMDataItem;
struct CommonToWidgetInfo;
class DMItemDataModelImpl;
class DMItemDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DMItemDataModel(QObject* parent = nullptr);
    ~DMItemDataModel() override;
    void setShowConvertData(QList<CommonToWidgetInfo>& fromList); /* 显示转换用的 */
    bool isValid() const;
    /* 设置单页的数量,默认是7 */
    void setSinglePagesCount(int count);
    int page() const;
    int totalCount() const;
    void changePage(int);
    void changePageUpdate(int);
    void calcPage();
    void setObjList(const QList<DynamicWidgetInfo>& objList); /* 用于查找数据和显示列表头 */
    void addItem(const DMDataItem& value);
    void updateItem(const DMDataItem&);
    void deleteItem(const QModelIndex& index);
    void deleteItem(const DMDataItem&);
    void updateData(const QList<DMDataItem>& logs);
    void updateData(const QMap<QString, DMDataItem>& logs);
    void updateData();
    bool dataExist(const QString& devID) const;
    bool dmDataItem(const QModelIndex& index, DMDataItem&);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool judgeData(int row);

    Optional<DMDataItem> getItem(const QModelIndex& index);

private:
    DMItemDataModelImpl* m_impl;
};

#endif  // DMITEMDATAMODEL_H
