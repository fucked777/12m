#ifndef ResourceRestructuringXMLReader_H
#define ResourceRestructuringXMLReader_H

#include <QDomElement>
#include <QList>
#include <QMap>
#include <QString>

struct Dev
{
    int id;
    QString desc;
    QString deviceType;
    QMap<int, QString> enums;
};

struct DeviceType
{
    QString id;
    QString desc;

    QList<Dev> devs;
};

struct DevicesType
{
    QString id;
    QString desc;

    QList<DeviceType> deviceTypes;
};

struct BBEConfig
{
    int id;
    QString desc;

    QList<DevicesType> devicesTypes;
};

class ResourceRestructuringXMLReader
{
public:
    ResourceRestructuringXMLReader();

    QList<BBEConfig> getBBEConfig();

    QMap<int, QString> getBBEIDAndType();

private:
    void parseResourceRestructingFile(const QString& filePath, QList<BBEConfig>& workSystems);
    void parseDevicesTypeNode(const QDomNode& workSysNode, QList<DevicesType>& devicesTypes);
    void parseDeviceTypeNode(const QDomNode& workSysNode, QList<DeviceType>& deviceTypes);
    void parseDevNode(const QDomNode& deviceTypeNode, QList<Dev>& devs);
    void parseEntryNode(const QDomNode& devNode, QMap<int, QString>& paramEnums);

private:
    QMap<int, QString> d_ID_Type;
};

#endif  // ResourceRestructuringXMLReader_H
