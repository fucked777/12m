#ifndef SYSTEMMAINSCREENSERVER_H
#define SYSTEMMAINSCREENSERVER_H

#include "DeviceProcessMessageDefine.h"
#include "INetMsg.h"
#include <QTime>

namespace cppmicroservices
{
    class BundleContext;
}

class SystemMainScreenServerImpl;
class SystemMainScreenServer : public INetMsg
{
    Q_OBJECT
public:
    SystemMainScreenServer(cppmicroservices::BundleContext context);
    ~SystemMainScreenServer();

signals:
    /*分机状态上报的数据*/
    void signalsStatusReportJson(StatusReportingRequest statusReport);
    /*分机过程控制命令结果上报 */
    void signalsDeviceCMDResult(const QByteArray& data);
    /*分机过程控制命令响应 */
    void signalsCMDResponceJson(const QByteArray& data);
public slots:
    // 发送过程控制命令
    void slotSendProcessCmd(QString json);

    void slotSendUnitParam(QString json);

    void slotManualFunction(QString json);

    /*接收设备过程控制命令响应 QByteArray*/
    void AcceptDeviceCMDResponceMessage(const ProtocolPack& pack);

    /*接收设备过程控制命令上报 QByteArray*/
    void AcceptDeviceCMDResultMessage(const ProtocolPack& pack);

private:
    SystemMainScreenServerImpl* mImpl;
    QString d_CurProjectId;
};

#endif  // SYSTEMMAINSCREENSERVER_H
