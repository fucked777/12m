#include "ExtendedConfig.h"
#include "PlatformInterface.h"
#include <QCoreApplication>
#include <QVariant>

QString ExtendedConfig::serverCommunicationIP()
{
    static auto gTempValue = PlatformConfigTools::valueStr("ServerCommunicationIP");
    return gTempValue;
}
quint16 ExtendedConfig::serverCommunicationPort()
{
    static auto gTempValue = PlatformConfigTools::valueStr("ServerCommunicationPort").toUShort();
    return gTempValue;
}
QString ExtendedConfig::serverCommunicationFile()
{
    static auto gTempValue = PlatformConfigTools::valueStr("ServerCommunicationFile");
    return gTempValue;
}
QString ExtendedConfig::clientCommunicationIP()
{
    static auto gTempValue = PlatformConfigTools::valueStr("ClientCommunicationIP");
    return gTempValue;
}
quint16 ExtendedConfig::clientCommunicationPort()
{
    static auto gTempValue = PlatformConfigTools::valueStr("ClientCommunicationPort").toUShort();
    return gTempValue;
}
QString ExtendedConfig::communicationInstanceID()
{
    static auto gTempValue = PlatformConfigTools::valueStr("CommunicationInstanceID");
    return gTempValue;
}
QString ExtendedConfig::curProjectID()
{
    static auto gTempValue = PlatformConfigTools::valueStr("CurProjectID");
    return gTempValue;
}
QString ExtendedConfig::curTKID()
{
    static auto gTempValue = QString("TK-%1").arg(curProjectID());
    return gTempValue;
}
