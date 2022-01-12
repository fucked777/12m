#ifndef SATELLITEMANAGEMENTSERIALIZE_H
#define SATELLITEMANAGEMENTSERIALIZE_H

#include "SatelliteManagementDefine.h"
#include "Utility.h"

/** 卫星每个模式的参数序列化 **/
JsonWriter& operator&(JsonWriter& self, const SatelliteManagementParamItem& entity);
JsonReader& operator&(JsonReader& self, SatelliteManagementParamItem& entity);

JsonWriter& operator&(JsonWriter& self, const QList<SatelliteManagementParamItem>& entity);
JsonReader& operator&(JsonReader& self, QList<SatelliteManagementParamItem>& entity);

JsonWriter& operator&(JsonWriter& self, const QMap<int, SatelliteManagementParamItem>& entity);
JsonReader& operator&(JsonReader& self, QMap<int, SatelliteManagementParamItem>& entity);

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity);
JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity);

QByteArray& operator>>(QByteArray& self, SatelliteManagementParamItem& entity);
QByteArray& operator<<(QByteArray& self, const SatelliteManagementParamItem& entity);
QString& operator<<(QString& self, const SatelliteManagementParamItem& entity);
QString& operator>>(QString& self, SatelliteManagementParamItem& entity);

QByteArray& operator>>(QByteArray& self, QList<SatelliteManagementParamItem>& entity);
QByteArray& operator<<(QByteArray& self, const QList<SatelliteManagementParamItem>& entity);
QString& operator<<(QString& self, const QList<SatelliteManagementParamItem>& entity);
QString& operator>>(QString& self, QList<SatelliteManagementParamItem>& entity);

QByteArray& operator>>(QByteArray& self, QMap<int, SatelliteManagementParamItem>& entity);
QByteArray& operator<<(QByteArray& self, const QMap<int, SatelliteManagementParamItem>& entity);
QString& operator<<(QString& self, const QMap<int, SatelliteManagementParamItem>& entity);
QString& operator>>(QString& self, QMap<int, SatelliteManagementParamItem>& entity);

QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity);
QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity);
QString& operator<<(QString& self, const QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity);
QString& operator>>(QString& self, QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity);

/** 卫星的结构体序列化json **/
JsonWriter& operator&(JsonWriter& self, const SatelliteManagementData& entity);
JsonReader& operator&(JsonReader& self, SatelliteManagementData& entity);

JsonWriter& operator&(JsonWriter& self, const QList<SatelliteManagementData>& entity);
JsonReader& operator&(JsonReader& self, QList<SatelliteManagementData>& entity);

JsonWriter& operator&(JsonWriter& self, const QMap<QString, SatelliteManagementData>& entity);
JsonReader& operator&(JsonReader& self, QMap<QString, SatelliteManagementData>& entity);

QByteArray& operator>>(QByteArray& self, SatelliteManagementData& entity);
QByteArray& operator<<(QByteArray& self, const SatelliteManagementData& entity);
QString& operator<<(QString& self, const SatelliteManagementData& entity);
QString& operator>>(QString& self, SatelliteManagementData& entity);

QByteArray& operator>>(QByteArray& self, QList<SatelliteManagementData>& entity);
QByteArray& operator<<(QByteArray& self, const QList<SatelliteManagementData>& entity);
QString& operator<<(QString& self, const QList<SatelliteManagementData>& entity);
QString& operator>>(QString& self, QList<SatelliteManagementData>& entity);

QByteArray& operator>>(QByteArray& self, QMap<QString, SatelliteManagementData>& entity);
QByteArray& operator<<(QByteArray& self, const QMap<QString, SatelliteManagementData>& entity);
QString& operator<<(QString& self, const QMap<QString, SatelliteManagementData>& entity);
QString& operator>>(QString& self, QMap<QString, SatelliteManagementData>& entity);

/** 卫星的结构体序列化json **/
JsonWriter& operator&(JsonWriter& self, const SatelliteManagementDpParamItem& entity);
JsonReader& operator&(JsonReader& self, SatelliteManagementDpParamItem& entity);

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity);
JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity);

QByteArray& operator>>(QByteArray& self, SatelliteManagementDpParamItem& entity);
QByteArray& operator<<(QByteArray& self, const SatelliteManagementDpParamItem& entity);
QString& operator<<(QString& self, const SatelliteManagementDpParamItem& entity);
QString& operator>>(QString& self, SatelliteManagementDpParamItem& entity);

QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity);
QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity);
QString& operator<<(QString& self, const QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity);
QString& operator>>(QString& self, QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity);

#endif  // SATELLITEMESSAGE_H
