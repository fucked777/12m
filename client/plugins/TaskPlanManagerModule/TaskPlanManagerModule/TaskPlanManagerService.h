#ifndef TASKPLANMANAGERSERVICE_H
#define TASKPLANMANAGERSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class TaskPlanManagerServiceImpl;
class TaskPlanManagerService : public INetMsg
{
    Q_OBJECT
public:
    TaskPlanManagerService(cppmicroservices::BundleContext context);
    ~TaskPlanManagerService();

signals:
    void sg_addItemACK(const QByteArray&);
    void sg_deletePlanItemACK(const QByteArray&);
    void sg_viewItemACK(const QByteArray&);
    void sg_editInfoItemACK(const QByteArray&);
    void sg_updateItemACK(const QByteArray&);
    // void sg_queryDeviceItemACK(const QByteArray&);
    // void sg_queryDTItemACK(const QByteArray&);

public:
    void addItem(const QByteArray&);
    void deletePlanItem(const QByteArray&);
    void viewItem(const QByteArray&);
    void editInfoItem(const QByteArray&);
    void updateItem(const QByteArray&);
    // void queryDeviceItem(const QByteArray&);
    // void queryDTItem(const QByteArray&);

    void addItemACK(const ProtocolPack&);
    void deletePlanItemACK(const ProtocolPack&);
    void viewItemACK(const ProtocolPack&);
    void editInfoItemACK(const ProtocolPack&);
    void updateItemACK(const ProtocolPack&);
    // void queryDeviceItemACK(const ProtocolPack&);
    // void queryDTItemACK(const ProtocolPack&);

private:
    TaskPlanManagerServiceImpl* m_impl;
};
#endif  // TASKPLANMANAGERSERVICE_H
