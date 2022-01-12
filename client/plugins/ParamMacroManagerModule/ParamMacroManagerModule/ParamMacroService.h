#ifndef ParamMacroSERVICE_H
#define ParamMacroSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class ParamMacroServiceImpl;
class ParamMacroService : public INetMsg
{
    Q_OBJECT
public:
    ParamMacroService(cppmicroservices::BundleContext context);
    ~ParamMacroService();

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
    ParamMacroServiceImpl* mImpl;
};
#endif  // ParamMacroSERVICE_H
