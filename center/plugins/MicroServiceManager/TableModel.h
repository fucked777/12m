#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
class MircoServiceInfo;
class TableModel : public QAbstractTableModel
{
    Q_OBJECT

    //    QString m_serviceID;           // current Macroservice ID  ,systemid group by 0
    //    QString m_symbolicName;        //
    //    QString m_serviceName;         // current Macroservice Name
    //    QString m_serviceVersion;      //
    //    QString m_serviceAuthors;      //
    //    QString m_serviceDescription;  //
    //    QString m_serviceLog;          // code update log
public:
    enum
    {
        SERVICE_ID,
        SERVICE_NAME,
        SYMBOLIC_NAME,
        SERVICE_VERSION,
        SERVICE_AUTHORS,
        SERVICE_DESCRIPTION,
        SERVCIE_UPDATE_LOG,
        NUM
    };

public:
    explicit TableModel(QObject* parent = 0);
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    void setHorizontalHeader(const QStringList& headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setData(const QList<MircoServiceInfo>& microServiceInfoList);
    QList<MircoServiceInfo>& DataVector() { return m_data; }
    ~TableModel(void);

    MircoServiceInfo rowData(int row);

signals:

public slots:

private:
    QStringList m_HorizontalHeader;
    QList<MircoServiceInfo> m_data;
};

#endif  // TABLEMODEL_H
