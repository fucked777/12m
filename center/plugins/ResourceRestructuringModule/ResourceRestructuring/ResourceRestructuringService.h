#ifndef ResourceRestructuringSERVICE_H
#define ResourceRestructuringSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class ResourceRestructuringServiceImpl;
class ResourceRestructuringService : public INetMsg
{
    Q_OBJECT
public:
    ResourceRestructuringService(cppmicroservices::BundleContext context);
    ~ResourceRestructuringService();

    void addItemACK(const ProtocolPack& pack);
    void deleteItemACK(const ProtocolPack& pack);
    void queryItemACK(const ProtocolPack& pack);
    void modifyItemACK(const ProtocolPack& pack);

signals:
    void signalAddItemACK(const QByteArray& data);
    void signalDeleteItemACK(const QByteArray& data);
    void signalQueryItemACK(const QByteArray& data);
    void signalModifyItemACK(const QByteArray& data);

public slots:
    void slotAddItem(const QByteArray& data);
    void slotDeleteItem(const QByteArray& data);
    void slotQueryItem(const QByteArray& data);
    void slotModifyItem(const QByteArray& data);

private:
    ResourceRestructuringServiceImpl* m_impl;
};
#endif  // CONFIGMICROSERVICE_H
