#ifndef TASKCENTERSERIALIZE_H
#define TASKCENTERSERIALIZE_H

#include "TaskCenterDefine.h"
#include "Utility.h"

class TCConvert
{
public:
    static QString tcdcToStr(TCDirectionType);
    static TCDirectionType tcdcFromStr(const QString&);
    static QString tcdcToDesStr(TCDirectionType);
    static QList<TCDirectionType> tcdcList(bool hasUnknown = false);                // 参数是是否包含未知
    static QList<EnumInfo<TCDirectionType>> tcdcListInfo(bool hasUnknown = false);  // 参数是是否包含未知

    static QString tcffToStr(TCFrameFormat);
    static TCFrameFormat tcffFromStr(const QString&);
    static QString tcffToDesStr(TCFrameFormat);
    static QList<TCFrameFormat> tcffList(bool hasUnknown = false);                // 参数是是否包含未知
    static QList<EnumInfo<TCFrameFormat>> tcffListInfo(bool hasUnknown = false);  // 参数是是否包含未知
};

JsonWriter& operator&(JsonWriter& self, const TaskCenterData& value);
JsonReader& operator&(JsonReader& self, TaskCenterData& value);
JsonWriter& operator&(JsonWriter& self, const TaskCenterMap& value);
JsonReader& operator&(JsonReader& self, TaskCenterMap& value);
JsonWriter& operator&(JsonWriter& self, const TaskCenterList& value);
JsonReader& operator&(JsonReader& self, TaskCenterList& value);

QByteArray& operator>>(QByteArray& self, TaskCenterData& value);
QByteArray& operator<<(QByteArray& self, const TaskCenterData& value);
QString& operator<<(QString& self, const TaskCenterData& value);
QString& operator>>(QString& self, TaskCenterData& value);

QByteArray& operator>>(QByteArray& self, TaskCenterMap& value);
QByteArray& operator<<(QByteArray& self, const TaskCenterMap& value);
QString& operator<<(QString& self, const TaskCenterMap& value);
QString& operator>>(QString& self, TaskCenterMap& value);

QByteArray& operator>>(QByteArray& self, TaskCenterList& value);
QByteArray& operator<<(QByteArray& self, const TaskCenterList& value);
QString& operator<<(QString& self, const TaskCenterList& value);
QString& operator>>(QString& self, TaskCenterList& value);

#endif  // TASKCENTERSERIALIZE_H
