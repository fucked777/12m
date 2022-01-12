#ifndef TASKPLANMESSAGESERIALIZE_H
#define TASKPLANMESSAGESERIALIZE_H

#include "TaskPlanMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const DataTranWorkTask& data);
JsonReader& operator&(JsonReader& self, DataTranWorkTask& data);
QByteArray& operator>>(QByteArray& self, DataTranWorkTask& data);
QByteArray& operator<<(QByteArray& self, const DataTranWorkTask& data);
QString& operator>>(QString& self, DataTranWorkTask& data);
QString& operator<<(QString& self, const DataTranWorkTask& data);

JsonWriter& operator&(JsonWriter& self, const QList<DataTranWorkTask>& data);
JsonReader& operator&(JsonReader& self, QList<DataTranWorkTask>& data);
QByteArray& operator>>(QByteArray& self, QList<DataTranWorkTask>& data);
QByteArray& operator<<(QByteArray& self, const QList<DataTranWorkTask>& data);
QString& operator>>(QString& self, QList<DataTranWorkTask>& data);
QString& operator<<(QString& self, const QList<DataTranWorkTask>& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskTargetLink& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskTargetLink& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskTargetLink& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskTargetLink& data);
QString& operator>>(QString& self, DeviceWorkTaskTargetLink& data);
QString& operator<<(QString& self, const DeviceWorkTaskTargetLink& data);

JsonWriter& operator&(JsonWriter& self, const QMap<int, DeviceWorkTaskTargetLink>& data);
JsonReader& operator&(JsonReader& self, QMap<int, DeviceWorkTaskTargetLink>& data);
QByteArray& operator>>(QByteArray& self, QMap<int, DeviceWorkTaskTargetLink>& data);
QByteArray& operator<<(QByteArray& self, const QMap<int, DeviceWorkTaskTargetLink>& data);
QString& operator>>(QString& self, QMap<int, DeviceWorkTaskTargetLink>& data);
QString& operator<<(QString& self, const QMap<int, DeviceWorkTaskTargetLink>& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskTarget& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskTarget& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskTarget& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskTarget& data);
QString& operator>>(QString& self, DeviceWorkTaskTarget& data);
QString& operator<<(QString& self, const DeviceWorkTaskTarget& data);

JsonWriter& operator&(JsonWriter& self, const QMap<int, DeviceWorkTaskTarget>& data);
JsonReader& operator&(JsonReader& self, QMap<int, DeviceWorkTaskTarget>& data);
QByteArray& operator>>(QByteArray& self, QMap<int, DeviceWorkTaskTarget>& data);
QByteArray& operator<<(QByteArray& self, const QMap<int, DeviceWorkTaskTarget>& data);
QString& operator>>(QString& self, QMap<int, DeviceWorkTaskTarget>& data);
QString& operator<<(QString& self, const QMap<int, DeviceWorkTaskTarget>& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTask& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTask& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTask& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTask& data);
QString& operator>>(QString& self, DeviceWorkTask& data);
QString& operator<<(QString& self, const DeviceWorkTask& data);

JsonWriter& operator&(JsonWriter& self, const QList<DeviceWorkTask>& data);
JsonReader& operator&(JsonReader& self, QList<DeviceWorkTask>& data);
QByteArray& operator>>(QByteArray& self, QList<DeviceWorkTask>& data);
QByteArray& operator<<(QByteArray& self, const QList<DeviceWorkTask>& data);
QString& operator>>(QString& self, QList<DeviceWorkTask>& data);
QString& operator<<(QString& self, const QList<DeviceWorkTask>& data);

JsonWriter& operator&(JsonWriter& self, const TaskPlanData& data);
JsonReader& operator&(JsonReader& self, TaskPlanData& data);
QByteArray& operator>>(QByteArray& self, TaskPlanData& data);
QByteArray& operator<<(QByteArray& self, const TaskPlanData& data);
QString& operator>>(QString& self, TaskPlanData& data);
QString& operator<<(QString& self, const TaskPlanData& data);

JsonWriter& operator&(JsonWriter& self, const QMap<QString, TaskPlanData>& data);
JsonReader& operator&(JsonReader& self, QMap<QString, TaskPlanData>& data);
QByteArray& operator>>(QByteArray& self, QMap<QString, TaskPlanData>& data);
QByteArray& operator<<(QByteArray& self, const QMap<QString, TaskPlanData>& data);
QString& operator>>(QString& self, QMap<QString, TaskPlanData>& data);
QString& operator<<(QString& self, const QMap<QString, TaskPlanData>& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskQuery& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskQuery& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskQuery& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskQuery& data);
QString& operator>>(QString& self, DeviceWorkTaskQuery& data);
QString& operator<<(QString& self, const DeviceWorkTaskQuery& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskConditionQuery& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskConditionQuery& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskConditionQuery& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskConditionQuery& data);
QString& operator>>(QString& self, DeviceWorkTaskConditionQuery& data);
QString& operator<<(QString& self, const DeviceWorkTaskConditionQuery& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskConditionQueryACK& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskConditionQueryACK& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskConditionQueryACK& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskConditionQueryACK& data);
QString& operator>>(QString& self, DeviceWorkTaskConditionQueryACK& data);
QString& operator<<(QString& self, const DeviceWorkTaskConditionQueryACK& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskUpdate& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskUpdate& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskUpdate& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskUpdate& data);
QString& operator>>(QString& self, DeviceWorkTaskUpdate& data);
QString& operator<<(QString& self, const DeviceWorkTaskUpdate& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskDelete& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskDelete& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskDelete& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskDelete& data);
QString& operator>>(QString& self, DeviceWorkTaskDelete& data);
QString& operator<<(QString& self, const DeviceWorkTaskDelete& data);

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskView& data);
JsonReader& operator&(JsonReader& self, DeviceWorkTaskView& data);
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskView& data);
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskView& data);
QString& operator>>(QString& self, DeviceWorkTaskView& data);
QString& operator<<(QString& self, const DeviceWorkTaskView& data);

JsonWriter& operator&(JsonWriter& self, const QMap<TaskStep, TaskStepStatus>& data);
JsonReader& operator&(JsonReader& self, QMap<TaskStep, TaskStepStatus>& data);
QByteArray& operator>>(QByteArray& self, QMap<TaskStep, TaskStepStatus>& data);
QByteArray& operator<<(QByteArray& self, const QMap<TaskStep, TaskStepStatus>& data);
QString& operator>>(QString& self, QMap<TaskStep, TaskStepStatus>& data);
QString& operator<<(QString& self, const QMap<TaskStep, TaskStepStatus>& data);

JsonWriter& operator&(JsonWriter& self, const CurrentRunningTaskPlanData& data);
JsonReader& operator&(JsonReader& self, CurrentRunningTaskPlanData& data);
QByteArray& operator>>(QByteArray& self, CurrentRunningTaskPlanData& data);
QByteArray& operator<<(QByteArray& self, const CurrentRunningTaskPlanData& data);
QString& operator>>(QString& self, CurrentRunningTaskPlanData& data);
QString& operator<<(QString& self, const CurrentRunningTaskPlanData& data);

#endif  // TASKPLANMESSAGESERIALIZE_H
