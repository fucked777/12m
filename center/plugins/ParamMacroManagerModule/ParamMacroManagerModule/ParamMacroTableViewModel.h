#ifndef PARAMMACROTABLEVIEWMODEL_H
#define PARAMMACROTABLEVIEWMODEL_H

#include <QAbstractTableModel>

#include "Utility.h"

struct ParamMacroData;
using ParamMacroDataList = QList<ParamMacroData>;

class ParamMacroTableViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        Name = 0,  // 参数宏名称
        WorkMode,  // 工作模式
        TaskCode,  // 任务代号
        Desc,      // 参数宏描述
        Operation  // 操作
    };

    explicit ParamMacroTableViewModel(QObject* parent = nullptr);
    ~ParamMacroTableViewModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setParamMacroData(const ParamMacroDataList& list);
    void addParamMacroData(const ParamMacroData& data);
    void deleteParamMacroData(const QModelIndex& index);
    void deleteParamMacroData(const ParamMacroData& data);
    void updateParamMacroData(const ParamMacroData& data);
    Optional<ParamMacroData> getParamMacroData(const QModelIndex& index) const;

private:
    QStringList mHeaders;
    ParamMacroDataList mParamMacroDatas;
};
#endif  // PARAMMACROTABLEVIEWMODEL_H
