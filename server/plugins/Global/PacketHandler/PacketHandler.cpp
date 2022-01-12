#include "PacketHandler.h"

#include "BasePacker.h"
#include "BaseUnpacker.h"
#include "CConverter.h"

PacketHandler& PacketHandler::instance()
{
    static PacketHandler packetHandler;
    return packetHandler;
}

PacketHandler::~PacketHandler()
{
    for (auto packer : mPackerMap)
    {
        if (packer != nullptr)
        {
            delete packer;
            packer = nullptr;
        }
    }

    for (auto unpacker : mUnpackerMap)
    {
        if (unpacker != nullptr)
        {
            delete unpacker;
            unpacker = nullptr;
        }
    }
}

bool PacketHandler::pack(const PackMessage& message, QByteArray& data, QString& errorMsg)
{
    // 获取组包器
    BasePacker* packer = mPackerMap.value(static_cast<quint32>(message.header.msgType));
    if (packer == nullptr)
    {
        return false;
    }

    // 组包
    if (!packer->pack(message, data, errorMsg))
    {
        return false;
    }

    return true;
}

bool PacketHandler::unpack(const QByteArray& data, UnpackMessage& message)
{
    // 获取信息类型
    DevMsgType msgType;
    if (!getMessageType(data, msgType))
    {
        return false;
    }

    // 获取解包器
    auto unpacker = mUnpackerMap.value(static_cast<quint32>(msgType));
    if (unpacker == nullptr)
    {
        return false;
    }

    // 解包
    if (!unpacker->unpack(data, message))
    {
        return false;
    }

    return true;
}

void PacketHandler::appendPacker(BasePacker* packer)
{
    if (packer != nullptr)
    {
        auto key = static_cast<quint32>(packer->messageType());

        auto oldPacker = mPackerMap.value(key);
        if (oldPacker != nullptr)
        {
            delete oldPacker;
            oldPacker = nullptr;
        }

        mPackerMap.insert(key, packer);
    }
}

BasePacker* PacketHandler::getPacker(const DevMsgType& msgType) { return mPackerMap.value(static_cast<quint32>(msgType), nullptr); }

void PacketHandler::appendUnpacker(BaseUnpacker* unpacker)
{
    if (unpacker != nullptr)
    {
        auto key = static_cast<quint32>(unpacker->messageType());

        auto oldUnpacker = mUnpackerMap.value(key);
        if (oldUnpacker != nullptr)
        {
            delete oldUnpacker;
            oldUnpacker = nullptr;
        }

        mUnpackerMap.insert(key, unpacker);
    }
}

BaseUnpacker* PacketHandler::getUnpacker(const DevMsgType& msgType) { return mUnpackerMap.value(static_cast<quint32>(msgType), nullptr); }

bool PacketHandler::getMessageType(const QByteArray& data, DevMsgType& msgType)
{
    if (data.length() < DevProtocolBIDIndex::value + DevProtocolBIDLength::value)
        return false;

    uint type = CConverter::fromByteArray<decltype(type)>(data.mid(DevProtocolBIDIndex::value, sizeof(type)));
    msgType = DevMsgType(type);

    return true;
}
