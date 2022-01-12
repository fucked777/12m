#ifndef STATIONRESMANAGEMENTSERVICE_H
#define STATIONRESMANAGEMENTSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class StationResManagementServiceImpl;
class StationResManagementService : public INetMsg
{
    Q_OBJECT
public:
    StationResManagementService(cppmicroservices::BundleContext context);
    ~StationResManagementService();

signals:
    /* 增删查改 回复 */
    void sg_addACK(const QByteArray&);
    void sg_deleteACK(const QByteArray&);
    void sg_modifyACK(const QByteArray&);

public:
    void addItem(const QByteArray& data);
    void deleteItem(const QByteArray& data);
    void modifyItem(const QByteArray& data);

    void addItemACK(const ProtocolPack& pack);
    void deleteItemACK(const ProtocolPack& pack);
    void modifyItemACK(const ProtocolPack& pack);

private:
    StationResManagementServiceImpl* m_impl;
};
#endif  // DATATRANSMISSIONCENTERSERVICE_H
