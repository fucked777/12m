#ifndef TASKCENTER_H
#define TASKCENTER_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class TaskCenterImpl;
class TaskCenter : public INetMsg
{
public:
    TaskCenter(cppmicroservices::BundleContext context);
    ~TaskCenter();

private:
    void start();
    void stop();
    void timerEvent(QTimerEvent* event) override;
    /* 重载设备管理数据
     * data 被忽略
     * ack Optional<TaskCenterMap>
     */
    void reload(const ProtocolPack& pack);
    /* 重载数据
     * data 被忽略
     * ack OptionalNotValue
     */
    void refresh(const ProtocolPack& pack);

    /* 添加一个设备
     * data DMDataItem
     * ack Optional<TaskCenterData>
     */
    void addItem(const ProtocolPack& pack);
    /* 删除一个设备
     * data DMDataItem
     * ack Optional<TaskCenterData>
     */
    void deleteItem(const ProtocolPack& pack);
    /* 查询
     * data 忽略
     * ack Optional<TaskCenterMap>
     */
    void queryItem(const ProtocolPack& pack);
    /* 更新
     * data DMDataItem
     * ack Optional<TaskCenterData>
     */
    void modifyItem(const ProtocolPack& pack);

private:
    TaskCenterImpl* m_impl;
};
#endif  // TASKCENTER_H
