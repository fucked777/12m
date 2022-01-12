#ifndef CONFIGMACROXMLWORKMODESERIALIZE_H
#define CONFIGMACROXMLWORKMODESERIALIZE_H

#include "ConfigMacroXmlWorkModeDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const EnumEntry& data);
JsonReader& operator&(JsonReader& self, EnumEntry& data);
QByteArray& operator>>(QByteArray& self, EnumEntry& data);
QByteArray& operator<<(QByteArray& self, const EnumEntry& data);
QString& operator>>(QString& self, EnumEntry& data);
QString& operator<<(QString& self, const EnumEntry& data);

JsonWriter& operator&(JsonWriter& self, const EnumEntryList& data);
JsonReader& operator&(JsonReader& self, EnumEntryList& data);
QByteArray& operator>>(QByteArray& self, EnumEntryList& data);
QByteArray& operator<<(QByteArray& self, const EnumEntryList& data);
QString& operator>>(QString& self, EnumEntryList& data);
QString& operator<<(QString& self, const EnumEntryList& data);

JsonWriter& operator&(JsonWriter& self, const Item& data);
JsonReader& operator&(JsonReader& self, Item& data);
QByteArray& operator>>(QByteArray& self, Item& data);
QByteArray& operator<<(QByteArray& self, const Item& data);
QString& operator>>(QString& self, Item& data);
QString& operator<<(QString& self, const Item& data);

JsonWriter& operator&(JsonWriter& self, const ItemList& data);
JsonReader& operator&(JsonReader& self, ItemList& data);
QByteArray& operator>>(QByteArray& self, ItemList& data);
QByteArray& operator<<(QByteArray& self, const ItemList& data);
QString& operator>>(QString& self, ItemList& data);
QString& operator<<(QString& self, const ItemList& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroWorkMode& data);
JsonReader& operator&(JsonReader& self, ConfigMacroWorkMode& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroWorkMode& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroWorkMode& data);
QString& operator>>(QString& self, ConfigMacroWorkMode& data);
QString& operator<<(QString& self, const ConfigMacroWorkMode& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroWorkModeList& data);
JsonReader& operator&(JsonReader& self, ConfigMacroWorkModeList& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroWorkModeList& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroWorkModeList& data);
QString& operator>>(QString& self, ConfigMacroWorkModeList& data);
QString& operator<<(QString& self, const ConfigMacroWorkModeList& data);

JsonWriter& operator&(JsonWriter& self, const ConfigMacroWorkModeMap& data);
JsonReader& operator&(JsonReader& self, ConfigMacroWorkModeMap& data);
QByteArray& operator>>(QByteArray& self, ConfigMacroWorkModeMap& data);
QByteArray& operator<<(QByteArray& self, const ConfigMacroWorkModeMap& data);
QString& operator>>(QString& self, ConfigMacroWorkModeMap& data);
QString& operator<<(QString& self, const ConfigMacroWorkModeMap& data);

#endif  // CONFIGMACROXMLWORKMODESERIALIZE_H
