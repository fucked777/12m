#ifndef SOFTUNITHOSTMODEL_H
#define SOFTUNITHOSTMODEL_H

#include <QAbstractItemModel>

struct SoftUnitHost
{
    qint32 runCode;    //软件单元运行主机编号
    qint32 runStatus;  //软件单元在主机上的运行状态
};

class SoftUnitHostModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit SoftUnitHostModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setHeaderData(const QStringList& headers);

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    void setData(const QList<SoftUnitHost>& softUnitHostList);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QStringList m_headerList;
    QList<SoftUnitHost> m_dataList;
};

#endif  // SOFTUNITHOSTMODEL_H
