#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class DeviceManagementImpl;
class DeviceManagement : public INetMsg
{
    Q_OBJECT
public:
    explicit DeviceManagement(cppmicroservices::BundleContext context);
    ~DeviceManagement() override;

private:
    void start();
    void stop();
    void timerEvent(QTimerEvent* event) override;
    /* 重载设备管理数据
     * data 被忽略
     * ack Optional<DMTypeMap>
     */
    void reload(const ProtocolPack& pack);
    /* 重载数据
     * data 被忽略
     * ack OptionalNotValue
     */
    void refresh(const ProtocolPack& pack);

    /* 添加一个设备
     * data DMDataItem
     * ack Optional<DMDataItem>
     */
    void addItem(const ProtocolPack& pack);
    /* 删除一个设备
     * data DMDataItem
     * ack Optional<DMDataItem>
     */
    void deleteItem(const ProtocolPack& pack);
    /* 查询
     * data DMDataItem设备类型为空是所有否则按照设备类型查询
     * ack Optional<DMTypeMap>
     */
    void queryItem(const ProtocolPack& pack);
    /* 更新
     * data DMDataItem
     * ack Optional<DMDataItem>
     */
    void modifyItem(const ProtocolPack& pack);

private:
    DeviceManagementImpl* m_impl;
};

#endif  // DEVICEMANAGER_H
