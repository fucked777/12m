#ifndef DMMESSAGESERIALIZE_H
#define DMMESSAGESERIALIZE_H
#include "DMMessageDefine.h"
#include "Utility.h"
#include <QByteArray>
#include <QString>

JsonWriter& operator&(JsonWriter& self, const DMDataItem& value);
JsonReader& operator&(JsonReader& self, DMDataItem& value);
JsonWriter& operator&(JsonWriter& self, const DMTypeMap& value);
JsonReader& operator&(JsonReader& self, DMTypeMap& value);
JsonWriter& operator&(JsonWriter& self, const DMTypeList& value);
JsonReader& operator&(JsonReader& self, DMTypeList& value);
JsonWriter& operator&(JsonWriter& self, const DMQuery& value);
JsonReader& operator&(JsonReader& self, DMQuery& value);

QByteArray& operator>>(QByteArray& self, DMDataItem& value);
QByteArray& operator<<(QByteArray& self, const DMDataItem& value);
QString& operator<<(QString& self, const DMDataItem& value);
QString& operator>>(QString& self, DMDataItem& value);

QByteArray& operator>>(QByteArray& self, DMTypeMap& value);
QByteArray& operator<<(QByteArray& self, const DMTypeMap& value);
QString& operator<<(QString& self, const DMTypeMap& value);
QString& operator>>(QString& self, DMTypeMap& value);

QByteArray& operator>>(QByteArray& self, DMTypeList& value);
QByteArray& operator<<(QByteArray& self, const DMTypeList& value);
QString& operator<<(QString& self, const DMTypeList& value);
QString& operator>>(QString& self, DMTypeList& value);

QByteArray& operator>>(QByteArray& self, DMQuery& value);
QByteArray& operator<<(QByteArray& self, const DMQuery& value);
QString& operator<<(QString& self, const DMQuery& value);
QString& operator>>(QString& self, DMQuery& value);

/* 这个类引入的目的是因为设备管理全是可配置的数据,保存的结果全是json
 * 有多重序列化的过程
 * 如果写在别的地方不好修改
 * 统一放在咋这里
 */
class DMMessageSerializeHelper
{
public:
    static void splitDMDataItemParam(const QByteArray& data, DMDataItem& value);
    static QByteArray joinDMDataItemParam(const DMDataItem& value);
};

#endif  // DMMESSAGESERIALIZE_H
