#ifndef DMSERVICE_H
#define DMSERVICE_H

#include "INetMsg.h"
struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

class DMServiceImpl;
class DMService : public INetMsg
{
    Q_OBJECT
public:
    DMService(cppmicroservices::BundleContext context);
    ~DMService();

signals:
    /* 增删查改 回复 */
    void sg_addACK(const QByteArray&);
    void sg_deleteACK(const QByteArray&);
    // void sg_queryACK(const QByteArray&);
    void sg_modifyACK(const QByteArray&);

public:
    void addItem(const QByteArray& data);
    void deleteItem(const QByteArray& data);
    // void queryItem(const QByteArray& data);
    void modifyItem(const QByteArray& data);

    void addItemACK(const ProtocolPack& pack);
    void deleteItemACK(const ProtocolPack& pack);
    // void queryItemACK(const ProtocolPack& pack);
    void modifyItemACK(const ProtocolPack& pack);

private:
    DMServiceImpl* m_impl;
};
#endif  // DMSERVICE_H
