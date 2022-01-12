#ifndef CONFIGMACROTABLEMODEL_H
#define CONFIGMACROTABLEMODEL_H

#include <QAbstractTableModel>

#include "Utility.h"

struct ConfigMacroData;
using ConfigMacroDataList = QList<ConfigMacroData>;
using ConfigMacroDataMap = QMap<QString, ConfigMacroData>;

class ConfigMacroTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        WorkMode = 0,  // 工作模式
        StationName,   // 站名
        IsMaster,      // 主备
        Desc           // 描述
    };

    explicit ConfigMacroTableModel(QObject* parent = nullptr);
    ~ConfigMacroTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setConfigMacroData(const ConfigMacroDataList& list);
    void addConfigMacroData(const ConfigMacroData& data);
    void deleteConfigMacroData(const QModelIndex& index);
    void deleteConfigMacroData(const ConfigMacroData& data);
    void updateConfigMacroData(const ConfigMacroData& data);
    Optional<ConfigMacroData> getConfigMacroData(const QModelIndex& index) const;

private:
    QStringList mHeaders;
    ConfigMacroDataList mConfigMacroDatas;
};

#endif  // CONFIGMACROTABLEMODEL_H
