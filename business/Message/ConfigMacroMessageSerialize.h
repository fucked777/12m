#ifndef CONFIGMACROMESSAGESERIALIZE2_H
#define CONFIGMACROMESSAGESERIALIZE2_H

#include "ConfigMacroMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmd& data);
JsonReader& operator&(JsonReader& self, ConfigMacroCmd& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroCmd& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmd& data);
QString& operator>>(QString& self, ConfigMacroCmd& data);
QString& operator<<(QString& self, const ConfigMacroCmd& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdMap& data);
JsonReader& operator&(JsonReader& self, ConfigMacroCmdMap& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdMap& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdMap& data);
QString& operator>>(QString& self, ConfigMacroCmdMap& data);
QString& operator<<(QString& self, const ConfigMacroCmdMap& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdList& data);
JsonReader& operator&(JsonReader& self, ConfigMacroCmdList& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdList& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdList& data);
QString& operator>>(QString& self, ConfigMacroCmdList& data);
QString& operator<<(QString& self, const ConfigMacroCmdList& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroModeItemMap& data);
JsonReader& operator&(JsonReader& self, ConfigMacroModeItemMap& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroModeItemMap& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroModeItemMap& data);
QString& operator>>(QString& self, ConfigMacroModeItemMap& data);
QString& operator<<(QString& self, const ConfigMacroModeItemMap& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroModeItemList& data);
JsonReader& operator&(JsonReader& self, ConfigMacroModeItemList& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroModeItemList& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroModeItemList& data);
QString& operator>>(QString& self, ConfigMacroModeItemList& data);
QString& operator<<(QString& self, const ConfigMacroModeItemList& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdModeMap& data);
JsonReader& operator&(JsonReader& self, ConfigMacroCmdModeMap& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdModeMap& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdModeMap& data);
QString& operator>>(QString& self, ConfigMacroCmdModeMap& data);
QString& operator<<(QString& self, const ConfigMacroCmdModeMap& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdModeList& data);
JsonReader& operator&(JsonReader& self, ConfigMacroCmdModeList& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdModeList& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdModeList& data);
QString& operator>>(QString& self, ConfigMacroCmdModeList& data);
QString& operator<<(QString& self, const ConfigMacroCmdModeList& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroData& data);
JsonReader& operator&(JsonReader& self, ConfigMacroData& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroData& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroData& data);
QString& operator>>(QString& self, ConfigMacroData& data);
QString& operator<<(QString& self, const ConfigMacroData& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroDataMap& data);
JsonReader& operator&(JsonReader& self, ConfigMacroDataMap& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroDataMap& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroDataMap& data);
QString& operator>>(QString& self, ConfigMacroDataMap& data);
QString& operator<<(QString& self, const ConfigMacroDataMap& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroDataList& data);
JsonReader& operator&(JsonReader& self, ConfigMacroDataList& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroDataList& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroDataList& data);
QString& operator>>(QString& self, ConfigMacroDataList& data);
QString& operator<<(QString& self, const ConfigMacroDataList& data);

class ConfigMacroMessageHelper
{
public:
    static QString createSearchName(const QSet<SystemWorkMode>&, MasterSlave masterSlave);
    static QString createSearchName(const SystemWorkMode&, MasterSlave masterSlave);
};

#endif  // CONFIGMACROMESSAGESERIALIZE2_H
