#ifndef DATATRANSMISSIONCENTER_H
#define DATATRANSMISSIONCENTER_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class DataTransmissionCenterImpl;
class DataTransmissionCenter : public INetMsg
{
public:
    DataTransmissionCenter(cppmicroservices::BundleContext context);
    ~DataTransmissionCenter();

private:
    void start();
    void stop();
    void timerEvent(QTimerEvent* event) override;
    /* 重载设备管理数据
     * data 被忽略
     * ack Optional<DataTransmissionCenterData>
     */
    void reload(const ProtocolPack& pack);
    /* 重载数据
     * data 被忽略
     * ack OptionalNotValue
     */
    void refresh(const ProtocolPack& pack);

    /* 添加一个设备
     * data DMDataItem
     * ack Optional<DataTransmissionCenterData>
     */
    void addItem(const ProtocolPack& pack);
    /* 删除一个设备
     * data DMDataItem
     * ack Optional<DataTransmissionCenterData>
     */
    void deleteItem(const ProtocolPack& pack);
    /* 查询
     * data 忽略
     * ack Optional<DataTransmissionCenterData>
     */
    void queryItem(const ProtocolPack& pack);
    /* 更新
     * data DMDataItem
     * ack Optional<DataTransmissionCenterData>
     */
    void modifyItem(const ProtocolPack& pack);

private:
    DataTransmissionCenterImpl* m_impl;
};
#endif  // DATATRANSMISSIONCENTER_H
