#ifndef AUTORUNPOLICY_H
#define AUTORUNPOLICY_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class AutorunPolicyImpl;
class AutorunPolicy : public INetMsg
{
    Q_OBJECT
public:
    explicit AutorunPolicy(cppmicroservices::BundleContext context);
    ~AutorunPolicy() override;

private:
    void start();
    void stop();
    void timerEvent(QTimerEvent* event) override;
    /* 更新
     * data AutorunPolicyData
     * ack Optional<AutorunPolicyData>
     */
    void modifyItem(const ProtocolPack& pack);

private:
    AutorunPolicyImpl* m_impl;
};

#endif  // AUTORUNPOLICY_H
