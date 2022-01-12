#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

struct HeartbeatMessage;
struct NetStatusItemInfo;
class NetWorkManagerImpl;
class NetWorkManager : public INetMsg
{
    Q_OBJECT
public:
    NetWorkManager(cppmicroservices::BundleContext context);
    ~NetWorkManager() override;

private:
    /* 停止所有网络通道 */
    void stopAll();
    void devStatusQuery();
    void timerEvent(QTimerEvent* event) override;
    /* 测试使用 模拟数据源 */
    void testDataSourcePush();
signals:
    /* 通道数据改变 */
    void sg_channelRecvDataChange(const NetStatusItemInfo&);
    void sg_channelSendDataChange(const NetStatusItemInfo&);

private:
    /* 向设备发送数据
     * data 发送给设备的数据(已经拼装好的完整协议)
     * ack OptionalNotValue
     */
    void sendData(const ProtocolPack& pack);

    /* 向设备发送数据 20210804 wp?
     * data 发送给别的设备的数据 协议未知 收到的数据固定为 ID(固定为2字节)+协议数据
     * ack OptionalNotValue
     */
    void sendOtherData(const ProtocolPack& pack);
    /* 重新加载,网络配置
     * data 忽略
     * ack OptionalNotValue
     */
    void reload(const ProtocolPack& pack);

    /* 设置当前发送队列的最大长度 默认是1000 */
    void setSendMaxLen(int max);

    /*
     * 将接收到的设备的数据转发到
     */
    void sendToPackManager(const QByteArray& data);

    /* 初始化定时查询命令的数据 */
    void initStatusQueryCmdPacker();
    void startStatusTimer();
    void stopStatusTimer();
    /* 主备切换 */
    void masterSlave(const HeartbeatMessage& msg);
private:
    NetWorkManagerImpl* m_impl;
};

#endif  // NETWORKMANAGER_H
