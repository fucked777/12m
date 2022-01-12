#ifndef PACKETHANDLER_H
#define PACKETHANDLER_H

#include "CustomPacketMessageDefine.h"

class BasePacker;
class BaseUnpacker;
class PacketHandler
{
public:
    static PacketHandler& instance();
    ~PacketHandler();

    bool pack(const PackMessage& message, QByteArray& data, QString& errorMsg);
    bool unpack(const QByteArray& data, UnpackMessage& message);

    void appendPacker(BasePacker* packer);
    BasePacker* getPacker(const DevMsgType& msgType);

    void appendUnpacker(BaseUnpacker* packer);
    BaseUnpacker* getUnpacker(const DevMsgType& msgType);

    // 解析消息类型
    static bool getMessageType(const QByteArray& data, DevMsgType& msgType);

private:
    QMap<quint32, BasePacker*> mPackerMap;
    QMap<quint32, BaseUnpacker*> mUnpackerMap;
};

#endif  // PACKETHANDLER_H
