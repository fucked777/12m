#ifndef CONFIGMICROSERVICE_H
#define CONFIGMICROSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class ConfigMacroServiceImpl;
class ConfigMacroService : public INetMsg
{
    Q_OBJECT
public:
    ConfigMacroService(cppmicroservices::BundleContext context);
    ~ConfigMacroService();

private:
    void addItemACK(const ProtocolPack& pack);
    void deleteItemACK(const ProtocolPack& pack);
    void modifyItemACK(const ProtocolPack& pack);

signals:
    void signalAddItemACK(const QByteArray& data);
    void signalDeleteItemACK(const QByteArray& data);
    void signalModifyItemACK(const QByteArray& data);

public slots:
    void slotAddItem(const QByteArray& data);
    void slotDeleteItem(const QByteArray& data);
    void slotModifyItem(const QByteArray& data);

private:
    ConfigMacroServiceImpl* mImpl;
};
#endif  // CONFIGMICROSERVICE_H
