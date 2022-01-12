#ifndef PARAMETERBINDINGMODEL_H
#define PARAMETERBINDINGMODEL_H

#include "AutomateTestUtility.h"
#include "Utility.h"
#include <QAbstractTableModel>
#include <QList>

struct ParameterBindingModelDataItem
{
    QVariantList varList;
    AutomateTestParameterBind bind;
};
class ParameterBindingModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ParameterBindingModel(QObject* parent = nullptr);
    void updateData(const AutomateTestParameterBindGroup&);
    void updateData(const QList<AutomateTestParameterBind>&);
    void updateData(const AutomateTestParameterBind&);
    void setIndexData(const ParameterBindingConfigInfo&);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool itemAt(AutomateTestParameterBind& item, const QModelIndex& index) const;

private:
    QList<ParameterBindingModelDataItem> records;
    QStringList headList;
    ParameterBindingConfigInfo bindingConfigInfo;
};

#endif  // PARAMETERBINDINGMODEL_H
