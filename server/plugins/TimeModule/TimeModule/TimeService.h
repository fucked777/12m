#ifndef TIMESERVICE_H
#define TIMESERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

struct HeartbeatMessage;
struct TimeData;
class TimeServiceImpl;
class TimeService : public INetMsg
{
public:
    TimeService(cppmicroservices::BundleContext context);
    ~TimeService();

public slots:
    // 保存时间配置信息到配置文件
    void slotSaveTimeConfigInfo(const ProtocolPack& pack);

private:
    void slotTimeUpdated(const TimeData& timeData);
    /* 主备切换 */
    void masterSlave(const HeartbeatMessage&msg);

private:
    TimeServiceImpl* mImpl = nullptr;
};
#endif  // TIMESERVICE_H
