#ifndef AGCANDSBFMODEL_H
#define AGCANDSBFMODEL_H
#include "AGCDefine.h"
#include <QAbstractListModel>
#include <QJsonObject>

class AGCAndSBFModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AGCAndSBFModel(QObject* parent = Q_NULLPTR);
    void addItem(const AGCResult&);
    void addItem(const QModelIndex& index);
    bool removeItem(const QModelIndex& index);
    void updateData(const QList<AGCResult>&);
    void updateData();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    AGCResultList value() { return m_records.result; }
    AGCCalibrationItem itemValue() { return m_records; }
    void setItemValue(const AGCCalibrationItem& temp);
    void resetItemValue();
    bool isChangeValue(); /* 是否改变过值了 */
    /* 启用或者禁用点编辑 */
    void setEnableEdit(bool isEnable);
    bool enableEdit() const;
    bool judgeData(const QModelIndex& index) const;
    bool isValid() const { return !m_records.taskCode.isEmpty(); }
    bool isEmpty() const { return m_records.result.isEmpty(); }

signals:
    void sg_dataChange();
    void sg_dataIsValid(bool);

private:
    Qt::ItemFlags m_flag;
    QStringList m_headerList;
    AGCCalibrationItem m_records;
    bool m_isChangeValue{ false };
};

#endif  // AGCANDSBFMODEL_H
