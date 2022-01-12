#ifndef TASKREPORTMESSAGESERIALIZE_H
#define TASKREPORTMESSAGESERIALIZE_H

#include "TaskReportMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const TaskReportTable& data);
JsonReader& operator&(JsonReader& self, TaskReportTable& data);
QByteArray& operator>>(QByteArray& self, TaskReportTable& data);
QByteArray& operator<<(QByteArray& self, const TaskReportTable& data);
QString& operator>>(QString& self, TaskReportTable& data);
QString& operator<<(QString& self, const TaskReportTable& data);

JsonWriter& operator&(JsonWriter& self, const TaskReportTableMap& data);
JsonReader& operator&(JsonReader& self, TaskReportTableMap& data);
QByteArray& operator>>(QByteArray& self, TaskReportTableMap& data);
QByteArray& operator<<(QByteArray& self, const TaskReportTableMap& data);
QString& operator>>(QString& self, TaskReportTableMap& data);
QString& operator<<(QString& self, const TaskReportTableMap& data);

JsonWriter& operator&(JsonWriter& self, const TaskReportTableList& data);
JsonReader& operator&(JsonReader& self, TaskReportTableList& data);
QByteArray& operator>>(QByteArray& self, TaskReportTableList& data);
QByteArray& operator<<(QByteArray& self, const TaskReportTableList& data);
QString& operator>>(QString& self, TaskReportTableList& data);
QString& operator<<(QString& self, const TaskReportTableList& data);

#endif  // TASKREPORTMESSAGESERIALIZE_H
