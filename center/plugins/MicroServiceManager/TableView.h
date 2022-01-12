#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "ButtonDelegate.h"
#include "TableModel.h"
#include "cppmicroservices/BundleContext.h"
#include <QTableView>
using namespace cppmicroservices;
class MircoServiceInfo;
class TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(QWidget* parent = 0);
    TableModel* tableModel() { return m_model; }

    ~TableView();
    void updateData(const QList<MircoServiceInfo>& microServiceInfoList);
    void setM_context(const BundleContext& m_context);

    void install(MircoServiceInfo info);
    void uninstall(MircoServiceInfo info);

signals:

public slots:

private:
    void iniData();

private:
    TableModel* m_model = nullptr;
    ButtonDelegate* m_buttonDelegate = nullptr;
    BundleContext m_context_;
};

#endif  // TABLEVIEW_H
