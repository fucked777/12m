#ifndef EPHEMERISDATA_H
#define EPHEMERISDATA_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class EphemerisDataImpl;
class EphemerisData : public INetMsg
{
public:
    EphemerisData(cppmicroservices::BundleContext context);
    ~EphemerisData();

private:
    void start();
    void stop();
    void timerEvent(QTimerEvent* event) override;
    /* 重载设备管理数据
     * data 被忽略
     * ack Optional<EphemerisDataData>
     */
    void reload(const ProtocolPack& pack);
    /* 重载数据
     * data 被忽略
     * ack OptionalNotValue
     */
    void refresh(const ProtocolPack& pack);

    /* 添加一个设备
     * data DMDataItem
     * ack Optional<EphemerisDataData>
     */
    void addItem(const ProtocolPack& pack);
    /* 删除一个设备
     * data DMDataItem
     * ack Optional<EphemerisDataData>
     */
    void deleteItem(const ProtocolPack& pack);
    /* 查询
     * data 忽略
     * ack Optional<EphemerisDataList>
     */
    void queryItem(const ProtocolPack& pack);
    /* 更新
     * data DMDataItem
     * ack Optional<EphemerisDataData>
     */
    void modifyItem(const ProtocolPack& pack);

private:
    EphemerisDataImpl* m_impl;
};
#endif  // EPHEMERISDATA_H
