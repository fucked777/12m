#ifndef DEVICECOMMUNICATIONCONFIG_H
#define DEVICECOMMUNICATIONCONFIG_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class DeviceCommunicationConfigImpl;
class DeviceCommunicationConfig : public INetMsg
{
public:
    DeviceCommunicationConfig(cppmicroservices::BundleContext context);
    ~DeviceCommunicationConfig();

private:
    void init();
    /* 重载配置文件
     * data 被忽略
     * ack QMap<quint32, CommunicationInfo>
     */
    void reload(const ProtocolPack& pack);
    /* 重载配置文件
     * data 被忽略
     * ack OptionalNotValue
     */
    void refresh(const ProtocolPack& pack);
    /* 获取所有的设备信息
     * data 被忽略
     * ack QMap<quint32, CommunicationInfo>
     */
    void getAll(const ProtocolPack& pack);
    /* 更新所有,会写入文件
     * data 是QMap<quint32, CommunicationInfo>
     * ack OptionalNotValue
     * 注意:如果数据为空则会清空所有数据
     */
    void changeAll(const ProtocolPack& pack);
    /* 更新指定ID的设备信息,会写入文件
     * data 是CommunicationInfo
     * ack OptionalNotValue
     */
    void changeInfoWithID(const ProtocolPack& pack);

private:
    DeviceCommunicationConfigImpl* m_impl;
};
#endif  // DEVICECONFIGCOMMUNICATION_H
