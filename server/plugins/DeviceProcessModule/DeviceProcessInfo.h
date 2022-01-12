#ifndef DEVICEPROCESSINFO_H
#define DEVICEPROCESSINFO_H

#include "Utility.h"
#include <QObject>
#include <QScopedPointer>

struct ProtocolPack;
struct PackMessage;
struct UnpackMessage;
struct UnitParamRequest;
struct CmdRequest;
struct ParamMacroRequest;
class DeviceProcessInfoImpl;
class DeviceProcessInfo : public QObject
{
    Q_OBJECT
    DIS_COPY_MOVE(DeviceProcessInfo)
public:
    DeviceProcessInfo(QObject* parent = nullptr);
    ~DeviceProcessInfo();

public:
    /*接收设备数据 QByteArray*/
    void acceptDeviceMessage(const ProtocolPack& pack);

    /*接收客户端单元参数设置命令*/
    void acceptClientUnitSettingMessage(const ProtocolPack& pack, const UnitParamRequest& paramRequest);

    /*接收客户端命令参数设置命令*/
    void acceptClientCmdSettingMessage(const ProtocolPack& pack, const CmdRequest& cmdRequest);

    /*接收客户端参数宏设置命令*/
    void acceptClientParamMacroSettingMessage(const ProtocolPack& pack, const ParamMacroRequest& paramRequest);

private:
    // 组包和推送待发送的命令信息
    void packAndPublishCmdInfo(const PackMessage& packMsg, const ProtocolPack& pack);

    // 发送分机状态状态到健康管理模块
    void sendStatusToHealthManager(const UnpackMessage& unpackMsg);

    // 处理分机状态上报
    void extensionStatusReport(const UnpackMessage& unpackMsg);
    // 处理控制命令响应
    void controlCmdResponse(const UnpackMessage& unpackMsg);
    // 处理控制结果上报
    void controlResultReport(const UnpackMessage& unpackMsg);

    void cmdResponcePush(const QString& key, const QString& ackString, const QByteArray& data, int timeMS);
signals:
    /* 发送数据给设备 */
    void sg_sendByteArrayToDevice(const QByteArray& array, const QString&);
    /* 命令的结果/响应 */
    void sg_cmdACK(const ProtocolPack& pack);


    /* 多线程使用 */
    void sg_acceptDeviceMessage(const ProtocolPack& pack);
    void sg_acceptClientUnitSettingMessage(const ProtocolPack& pack, const UnitParamRequest& paramRequest);
    void sg_acceptClientCmdSettingMessage(const ProtocolPack& pack, const CmdRequest& cmdRequest);
    void sg_acceptClientParamMacroSettingMessage(const ProtocolPack& pack, const ParamMacroRequest& paramRequest);
private:
    QScopedPointer<DeviceProcessInfoImpl> m_impl;
};
#endif  // DEVICEPROCESSINFO_H
