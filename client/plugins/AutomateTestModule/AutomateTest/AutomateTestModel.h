#ifndef AUTOMATETESTMODEL_H
#define AUTOMATETESTMODEL_H

#include "AutomateTestDefine.h"
#include <QAbstractListModel>

struct AutomateTestModelInfo
{
    QString testName;     /* 测试计划名称 */
    QString workMode;     /* 工作模式 */
    QString testTypeName; /* 测试类型 */
    QString groupName;    /* 测试分系统 */

    AutomateTestPlanItem automateTestPlanItem;
};

/*
 * 20210526 这里考虑的是这个测试计划不会很多每次直接全部查询过来就行
 */
class AutomateTestModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AutomateTestModel(QObject* parent = Q_NULLPTR);
    bool isValid();
    void setTestItemData(const QList<AutomateTestPlanItem>&);
    void clear();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool testItem(const QModelIndex& index, AutomateTestPlanItem& item);

private:
    QList<AutomateTestModelInfo> records;
    QStringList headerList;
};

#endif  // AUTOMATETESTMODEL_H
