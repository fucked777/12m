#ifndef AUTOTASK4424SERVICE_H
#define AUTOTASK4424SERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

struct ManualMessage;
class AutoTask4424Service : public INetMsg
{
public:
    AutoTask4424Service(cppmicroservices::BundleContext context);
    ~AutoTask4424Service();

private:
    // 手动控制
    void manualOperation(const ProtocolPack& pack);

private slots:
    // 发送数据给设备
    void slotSendByteArrayToDevice(const QByteArray& data);
    // 推送提示消息
    void slotPublishInfoMsg(const QString& msg);
    // 推送警告消息
    void slotPublishWarningMsg(const QString& msg);
    // 推送错误消息
    void slotPublishErrorMsg(const QString& msg);

private:
    QString UserID;
};

#endif  // AUTOTASK4424SERVICE_H
