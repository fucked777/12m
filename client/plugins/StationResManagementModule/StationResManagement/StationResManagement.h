#ifndef DATATRANSMISSIONCENTER_H
#define DATATRANSMISSIONCENTER_H

#include "StationResManagementModel.h"
#include <QMenu>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class StationResManagement;
}
QT_END_NAMESPACE
namespace cppmicroservices
{
    class BundleContext;
}

struct StationResManagementData;
class StationResManagementImpl;
class StationResManagement : public QWidget
{
    Q_OBJECT

public:
    explicit StationResManagement(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~StationResManagement();

public:
    void deleteACK(const QByteArray&);

private:
    void showEvent(QShowEvent*) override;
    void init();

    void addItem();
    void deleteItem();
    void queryItem();
    void modifyItem();

signals:
    /* 增删改 */
    void sg_add(const QByteArray&);
    void sg_delete(const QByteArray&);
    void sg_modify(const QByteArray&);

    void sg_addACK(const QByteArray&);
    void sg_modifyACK(const QByteArray&);

private:
    Ui::StationResManagement* ui;
    StationResManagementImpl* m_impl;
};

#endif  // DATATRANSMISSIONCENTER_H
