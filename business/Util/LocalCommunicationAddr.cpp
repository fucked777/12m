#include "LocalCommunicationAddr.h"
#include "CConverter.h"
#include "DevProtocolSerialize.h"
#include "PlatformInterface.h"
#include <QCoreApplication>
#include <QVariant>

void LocalCommunicationAddr::setDevAddrMessage(const MessageAddress& addr)
{
    PlatformConfigTools::setValue("BaseID", static_cast<qint32>(addr.baseNumb));
    PlatformConfigTools::setValue("StationID", static_cast<qint32>(addr.stationNumb));
    PlatformConfigTools::setValue("DeviceID", static_cast<qint32>(addr.equipmentNumb));

#ifndef COMPATIBLE
    PlatformConfigTools::setValue("ExtenName", static_cast<qint32>(addr.extenName));
#else
    auto temp = addr.systemNumb * 0x10 + addr.deviceNumb;
    PlatformConfigTools::setValue("ExtenName", static_cast<qint32>(temp));
#endif

    PlatformConfigTools::setValue("ExtenNumb", static_cast<qint32>(addr.extenNumb));
}
void LocalCommunicationAddr::setDevAddrProtocol(quint64 rawAddr)
{
    auto array = CConverter::toByteArray<quint64>(rawAddr);
    MessageAddress addr;
    array >> addr;
    setDevAddrMessage(addr);
}

quint8 LocalCommunicationAddr::baseID()
{
    static auto gTempValue = static_cast<quint8>(PlatformConfigTools::valueStr("BaseID").toUShort());
    return gTempValue;
}
quint8 LocalCommunicationAddr::stationID()
{
    static auto gTempValue = static_cast<quint8>(PlatformConfigTools::valueStr("StationID").toUShort());
    return gTempValue;
}
quint8 LocalCommunicationAddr::equipmentID()
{
    static auto gTempValue = static_cast<quint8>(PlatformConfigTools::valueStr("DeviceID").toUShort());
    return gTempValue;
}
quint8 LocalCommunicationAddr::extenName()
{
    static auto gTempValue = static_cast<quint8>(PlatformConfigTools::valueStr("ExtenName").toUShort());
    return gTempValue;
}
quint8 LocalCommunicationAddr::systemID()
{
    static auto gTempValue = extenName() / 0x10;
    return gTempValue;
}
quint8 LocalCommunicationAddr::deviceID()
{
    static auto gTempValue = extenName() % 0x10;
    return gTempValue;
}
quint8 LocalCommunicationAddr::extenNumb()
{
    static auto gTempValue = static_cast<quint8>(PlatformConfigTools::valueStr("ExtenNumb").toUShort());
    return gTempValue;
}
quint32 LocalCommunicationAddr::addrPrefix()
{
    static auto gTempValue =
        (static_cast<quint32>(baseID()) << (8 * 2)) | (static_cast<quint32>(stationID()) << (8 * 1)) | (static_cast<quint32>(equipmentID()));
    return gTempValue;
}
quint32 LocalCommunicationAddr::devID()
{
    static auto gTempValue = (static_cast<quint32>(extenName()) << (8 * 1)) | (static_cast<quint32>(extenNumb()));
    return gTempValue;
}
quint64 LocalCommunicationAddr::devAddrProtocol()
{
    static auto gTempValue = (static_cast<quint64>(baseID()) << (8 * 7)) | (static_cast<quint64>(stationID()) << (8 * 6)) |
                             (static_cast<quint64>(equipmentID()) << (8 * 5)) | (static_cast<quint64>(extenName()) << (8 * 4)) |
                             (static_cast<quint64>(extenNumb()) << (8 * 3));

    return gTempValue;
}
MessageAddress devAddrMessageImpl()
{
    MessageAddress addr;
    addr.baseNumb = LocalCommunicationAddr::baseID();
    addr.stationNumb = LocalCommunicationAddr::stationID();
    addr.equipmentNumb = LocalCommunicationAddr::equipmentID();
#ifdef COMPATIBLE
    addr.systemNumb = LocalCommunicationAddr::systemID();
    addr.deviceNumb = LocalCommunicationAddr::deviceID();
#else
    addr.extenName = LocalCommunicationAddr::extenName();
#endif
    addr.extenNumb = LocalCommunicationAddr::extenNumb();

    return addr;
}
MessageAddress LocalCommunicationAddr::devAddrMessage()
{
    static auto gTempValue = devAddrMessageImpl();
    return gTempValue;
}
