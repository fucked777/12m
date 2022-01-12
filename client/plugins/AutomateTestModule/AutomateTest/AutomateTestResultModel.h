#ifndef AUTOMATETESTRESULTMODEL_H
#define AUTOMATETESTRESULTMODEL_H
#include "AutomateTestDefine.h"
#include "Utility.h"
#include <QAbstractListModel>

class AutomateTestResultModelImpl;
class AutomateTestResultModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AutomateTestResultModel(QObject* parent = nullptr);
    void updateData(const AutomateTestResultList& logs);
    void removeValue(const AutomateTestResult& removeItem);
    void clear();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Optional<AutomateTestResult> getItem(const QModelIndex& index) const;

private:
    AutomateTestResultList records;
    QStringList headerList;
};

#endif  // AUTOMATETESTRESULTMODEL_H
