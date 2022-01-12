#ifndef AUTORUNPOLICYSERVICE_H
#define AUTORUNPOLICYSERVICE_H

#include "INetMsg.h"
struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

class AutorunPolicyServiceImpl;
class AutorunPolicyService : public INetMsg
{
    Q_OBJECT
public:
    AutorunPolicyService(cppmicroservices::BundleContext context);
    ~AutorunPolicyService();

signals:
    /* 自动化运行策略只有修改 */
    void sg_modifyACK(const QByteArray&);

public:
    void modifyItem(const QByteArray& data);
    void modifyItemACK(const ProtocolPack& pack);

private:
    AutorunPolicyServiceImpl* m_impl;
};
#endif  // AUTORUNPOLICYSERVICE_H
