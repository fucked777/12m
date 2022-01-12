#include "TaskPlanMessageSerialize.h"

#include "PlanRunMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const DataTranWorkTask& data)
{
    self.StartObject();

    self.Member("Uuid") & data.m_uuid;
    self.Member("CreateUserId") & data.m_create_userid;
    self.Member("CreateTime") & data.m_createTime;
    self.Member("TaskStartTime") & data.m_task_start_time;
    self.Member("TaskEndTime") & data.m_task_end_time;
    self.Member("TransportTime") & data.m_transport_time;
    self.Member("TaskCommitTime") & data.m_task_commit_time;
    self.Member("SerialNumber") & data.m_plan_serial_number;
    self.Member("WorkUnit") & data.m_work_unit;
    self.Member("EquipmentNo") & data.m_equipment_no;
    self.Member("TaskTotalNum") & data.m_task_total_num;
    self.Member("TaskStatus") & data.m_task_status;
    self.Member("WorkMode") & data.m_working_mode;
    self.Member("TaskCode") & data.m_task_code;
    self.Member("DTTransNum") & data.m_dt_transNum;
    self.Member("ChannelNum") & data.m_channelNum;
    self.Member("DataTransCenter") & data.m_datatrans_center;
    self.Member("DataTransCenterProtocol") & data.m_center_protocol;
    self.Member("DataTransCenterId") & data.m_datatrans_center_id;
    self.Member("Priority") & data.m_priority;
    self.Member("TransportProtocol") & data.m_transport_protocols;
    self.Member("RingNum") & data.m_ring_num;
    self.Member("Source") & data.m_source;
    self.Member("BelongTarget") & data.m_belongTarget;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DataTranWorkTask& data)
{
    self.StartObject();

    self.Member("Uuid") & data.m_uuid;
    self.Member("CreateUserId") & data.m_create_userid;
    self.Member("CreateTime") & data.m_createTime;
    self.Member("TaskStartTime") & data.m_task_start_time;
    self.Member("TaskEndTime") & data.m_task_end_time;
    self.Member("TransportTime") & data.m_transport_time;
    self.Member("TaskCommitTime") & data.m_task_commit_time;
    self.Member("SerialNumber") & data.m_plan_serial_number;
    self.Member("WorkUnit") & data.m_work_unit;
    self.Member("EquipmentNo") & data.m_equipment_no;
    self.Member("TaskTotalNum") & data.m_task_total_num;
    self.Member("TaskStatus") & data.m_task_status;
    self.Member("WorkMode") & data.m_working_mode;
    self.Member("TaskCode") & data.m_task_code;
    self.Member("DTTransNum") & data.m_dt_transNum;
    self.Member("ChannelNum") & data.m_channelNum;
    self.Member("DataTransCenter") & data.m_datatrans_center;
    self.Member("DataTransCenterProtocol") & data.m_center_protocol;
    self.Member("DataTransCenterId") & data.m_datatrans_center_id;
    self.Member("Priority") & data.m_priority;
    self.Member("TransportProtocol") & data.m_transport_protocols;
    self.Member("RingNum") & data.m_ring_num;
    self.Member("Source") & data.m_source;
    self.Member("BelongTarget") & data.m_belongTarget;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DataTranWorkTask& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const DataTranWorkTask& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DataTranWorkTask& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const DataTranWorkTask& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QList<DataTranWorkTask>& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }

    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QList<DataTranWorkTask>& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        DataTranWorkTask item;
        self& item;
        data << item;
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QList<DataTranWorkTask>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QList<DataTranWorkTask>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QList<DataTranWorkTask>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QList<DataTranWorkTask>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskTargetLink& data)
{
    self.StartObject();
    self.Member("WorkPointFreq") & data.working_point_frequency;
    self.Member("WorkMode") & data.m_work_system;
    self.Member("CodeGroupYC") & data.m_codeGroup_yc;
    self.Member("CodeGroupYK") & data.m_codeGroup_yk;
    self.Member("CodeGroupSXCJ") & data.m_codeGroup_sxcj;
    self.Member("CodeGroupXXCJ") & data.m_codeGroup_xxcj;
    self.Member("TaskTypeYC") & data.m_taskType_yc;
    self.Member("TaskTypeYK") & data.m_taskType_yk;
    self.Member("TaskTypeCG") & data.m_taskType_cg;
    self.Member("TaskTypeSC") & data.m_taskType_sc;
    self.Member("BasebandNumber") & data.m_baseband_number;
    self.Member("LinkVal") & data.m_linkVal;
    self.Member("CarrierType") & data.m_carrierType;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceWorkTaskTargetLink& data)
{
    self.StartObject();
    self.Member("WorkPointFreq") & data.working_point_frequency;
    self.Member("WorkMode") & data.m_work_system;
    self.Member("CodeGroupYC") & data.m_codeGroup_yc;
    self.Member("CodeGroupYK") & data.m_codeGroup_yk;
    self.Member("CodeGroupSXCJ") & data.m_codeGroup_sxcj;
    self.Member("CodeGroupXXCJ") & data.m_codeGroup_xxcj;
    self.Member("TaskTypeYC") & data.m_taskType_yc;
    self.Member("TaskTypeYK") & data.m_taskType_yk;
    self.Member("TaskTypeCG") & data.m_taskType_cg;
    self.Member("TaskTypeSC") & data.m_taskType_sc;
    self.Member("BasebandNumber") & data.m_baseband_number;
    self.Member("LinkVal") & data.m_linkVal;
    self.Member("CarrierType") & data.m_carrierType;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceWorkTaskTargetLink& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskTargetLink& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceWorkTaskTargetLink& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const DeviceWorkTaskTargetLink& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<int, DeviceWorkTaskTargetLink>& data)
{
    self.StartObject();

    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, DeviceWorkTaskTargetLink>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto& key : keys)
    {
        DeviceWorkTaskTargetLink link;
        self.Member(key) & link;
        data[key.toInt()] = link;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<int, DeviceWorkTaskTargetLink>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, DeviceWorkTaskTargetLink>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<int, DeviceWorkTaskTargetLink>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QMap<int, DeviceWorkTaskTargetLink>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskTarget& data)
{
    self.StartObject();
    self.Member("TaskCode") & data.m_task_code;
    self.Member("DTTransNum") & data.m_dt_transNum;
    self.Member("CircleNo") & data.m_circle_no;
    self.Member("TaskReadyStartTime") & data.m_task_ready_start_time;
    self.Member("TaskStartTime") & data.m_task_start_time;
    self.Member("TrackStartTime") & data.m_track_start_time;
    self.Member("RemoteCtrlStartTime") & data.m_remote_start_time;
    self.Member("RemoteCtrlEndTime") & data.m_remote_control_end_time;
    self.Member("UpStreamSignalStartTime") & data.m_on_uplink_signal_time;
    self.Member("UpStreamSignalEndTime") & data.m_turn_off_line_signal_time;
    self.Member("DTStartTime") & data.m_dt_starttime;
    self.Member("DTEndTime") & data.m_dt_endtime;
    self.Member("TrackEndTime") & data.m_track_end_time;
    self.Member("TaskEndTime") & data.m_task_end_time;
    self.Member("MasterCtrlCenter") & data.m_master_control_center;
    self.Member("BackupCenter") & data.m_backup_center;
    self.Member("PlanType") & data.m_planType;
    self.Member("LinkNumber") & data.m_link_number;

    self.Member("LinkMap") & data.m_linkMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceWorkTaskTarget& data)
{
    self.StartObject();
    self.Member("TaskCode") & data.m_task_code;
    self.Member("DTTransNum") & data.m_dt_transNum;
    self.Member("CircleNo") & data.m_circle_no;
    self.Member("TaskReadyStartTime") & data.m_task_ready_start_time;
    self.Member("TaskStartTime") & data.m_task_start_time;
    self.Member("TrackStartTime") & data.m_track_start_time;
    self.Member("RemoteCtrlStartTime") & data.m_remote_start_time;
    self.Member("RemoteCtrlEndTime") & data.m_remote_control_end_time;
    self.Member("UpStreamSignalStartTime") & data.m_on_uplink_signal_time;
    self.Member("UpStreamSignalEndTime") & data.m_turn_off_line_signal_time;
    self.Member("DTStartTime") & data.m_dt_starttime;
    self.Member("DTEndTime") & data.m_dt_endtime;
    self.Member("TrackEndTime") & data.m_track_end_time;
    self.Member("TaskEndTime") & data.m_task_end_time;
    self.Member("MasterCtrlCenter") & data.m_master_control_center;
    self.Member("BackupCenter") & data.m_backup_center;
    self.Member("PlanType") & data.m_planType;
    self.Member("LinkNumber") & data.m_link_number;

    self.Member("LinkMap") & data.m_linkMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceWorkTaskTarget& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskTarget& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceWorkTaskTarget& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceWorkTaskTarget& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<int, DeviceWorkTaskTarget>& data)
{
    self.StartObject();

    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, DeviceWorkTaskTarget>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto& key : keys)
    {
        DeviceWorkTaskTarget target;
        self.Member(key) & target;
        data[key.toInt()] = target;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<int, DeviceWorkTaskTarget>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, DeviceWorkTaskTarget>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<int, DeviceWorkTaskTarget>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QMap<int, DeviceWorkTaskTarget>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTask& data)
{
    self.StartObject();
    self.Member("Uuid") & data.m_uuid;
    self.Member("CreateUserId") & data.m_create_userid;
    self.Member("CreateTime") & data.m_createTime;
    self.Member("ReadyStartTime") & data.m_preStartTime;
    self.Member("StartTime") & data.m_startTime;
    self.Member("EndTime") & data.m_endTime;
    self.Member("SerialNumber") & data.m_plan_serial_number;
    self.Member("WorkUnit") & data.m_work_unit;
    self.Member("EquipmentNo") & data.m_equipment_no;
    self.Member("SchedulingMode") & data.m_scheduling_mode;
    self.Member("TaskTotalNum") & data.m_task_total_num;
    self.Member("IsZeroCorrection") & data.m_is_zero;
    self.Member("IsPhaseCorrection") & data.m_calibration;
    self.Member("CurrentTaskName") & data.m_currentTaskName;
    self.Member("WorkMode") & data.m_working_mode;
    self.Member("TargetNumber") & data.m_target_number;
    self.Member("MainTrackTargetTaskCode") & data.m_lord_with_target;
    self.Member("TrackOrder") & data.m_tracking_order;
    self.Member("TaskStatus") & data.m_task_status;
    self.Member("Source") & data.m_source;

    self.Member("TargetMap") & data.m_totalTargetMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceWorkTask& data)
{
    self.StartObject();
    self.Member("Uuid") & data.m_uuid;
    self.Member("CreateUserId") & data.m_create_userid;
    self.Member("CreateTime") & data.m_createTime;
    self.Member("ReadyStartTime") & data.m_preStartTime;
    self.Member("StartTime") & data.m_startTime;
    self.Member("EndTime") & data.m_endTime;
    self.Member("SerialNumber") & data.m_plan_serial_number;
    self.Member("WorkUnit") & data.m_work_unit;
    self.Member("EquipmentNo") & data.m_equipment_no;
    self.Member("SchedulingMode") & data.m_scheduling_mode;
    self.Member("TaskTotalNum") & data.m_task_total_num;
    self.Member("IsZeroCorrection") & data.m_is_zero;
    self.Member("IsPhaseCorrection") & data.m_calibration;
    self.Member("CurrentTaskName") & data.m_currentTaskName;
    self.Member("WorkMode") & data.m_working_mode;
    self.Member("TargetNumber") & data.m_target_number;
    self.Member("MainTrackTargetTaskCode") & data.m_lord_with_target;
    self.Member("TrackOrder") & data.m_tracking_order;
    self.Member("TaskStatus") & data.m_task_status;
    self.Member("Source") & data.m_source;

    self.Member("TargetMap") & data.m_totalTargetMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceWorkTask& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceWorkTask& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceWorkTask& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceWorkTask& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QList<DeviceWorkTask>& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }

    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QList<DeviceWorkTask>& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        DeviceWorkTask item;
        self& item;
        data << item;
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QList<DeviceWorkTask>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QList<DeviceWorkTask>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QList<DeviceWorkTask>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QList<DeviceWorkTask>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const TaskPlanData& data)
{
    self.StartObject();

    self.Member("DeviceWorkTask") & data.deviceWorkTask;
    self.Member("DataTransWorkTasks") & data.dataTransWorkTasks;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, TaskPlanData& data)
{
    self.StartObject();

    self.Member("DeviceWorkTask") & data.deviceWorkTask;
    self.Member("DataTransWorkTasks") & data.dataTransWorkTasks;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TaskPlanData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const TaskPlanData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TaskPlanData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const TaskPlanData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<QString, TaskPlanData>& data)
{
    self.StartObject();

    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<QString, TaskPlanData>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto& key : keys)
    {
        TaskPlanData taskPlan;
        self.Member(key) & taskPlan;
        data[key] = taskPlan;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<QString, TaskPlanData>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<QString, TaskPlanData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<QString, TaskPlanData>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QMap<QString, TaskPlanData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskQuery& data)
{
    self.StartObject();
    self.Member("beginTime") & data.beginDate;
    self.Member("endTime") & data.endDate;
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceWorkTaskQuery& data)
{
    self.StartObject();
    self.Member("beginTime") & data.beginDate;
    self.Member("endTime") & data.endDate;
    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceWorkTaskQuery& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceWorkTaskQuery& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const DeviceWorkTaskQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskUpdate& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;
    self.Member("planStatus") & data.planStatus;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DeviceWorkTaskUpdate& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;
    self.Member("planStatus") & data.planStatus;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskUpdate& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskUpdate& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, DeviceWorkTaskUpdate& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QString& operator<<(QString& self, const DeviceWorkTaskUpdate& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskDelete& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DeviceWorkTaskDelete& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskDelete& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskDelete& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, DeviceWorkTaskDelete& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QString& operator<<(QString& self, const DeviceWorkTaskDelete& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskView& data)
{
    self.StartObject();
    self.Member("sn") & data.sn;
    self.Member("id") & data.id;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DeviceWorkTaskView& data)
{
    self.StartObject();
    self.Member("sn") & data.sn;
    self.Member("id") & data.id;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, DeviceWorkTaskView& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskView& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, DeviceWorkTaskView& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QString& operator<<(QString& self, const DeviceWorkTaskView& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskConditionQuery& data)
{
    self.StartObject();
    self.Member("beginTime") & data.beginDate;
    self.Member("endTime") & data.endDate;
    self.Member("taskStatus") & data.taskStatus;
    self.Member("totalCount") & data.totalCount;
    self.Member("currentPage") & data.currentPage;
    self.Member("pageSize") & data.pageSize;
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceWorkTaskConditionQuery& data)
{
    self.StartObject();
    self.Member("beginTime") & data.beginDate;
    self.Member("endTime") & data.endDate;
    self.Member("taskStatus") & data.taskStatus;
    self.Member("totalCount") & data.totalCount;
    self.Member("currentPage") & data.currentPage;
    self.Member("pageSize") & data.pageSize;
    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceWorkTaskConditionQuery& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskConditionQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceWorkTaskConditionQuery& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const DeviceWorkTaskConditionQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceWorkTaskConditionQueryACK& data)
{
    self.StartObject();

    self.Member("totalCount") & data.totalCount;
    self.Member("currentPage") & data.currentPage;
    self.Member("DeviceWorkTasks") & data.deviceWorkTasks;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceWorkTaskConditionQueryACK& data)
{
    self.StartObject();

    self.Member("totalCount") & data.totalCount;
    self.Member("currentPage") & data.currentPage;
    self.Member("DeviceWorkTasks") & data.deviceWorkTasks;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceWorkTaskConditionQueryACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceWorkTaskConditionQueryACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceWorkTaskConditionQueryACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceWorkTaskConditionQueryACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<TaskStep, TaskStepStatus>& data)
{
    self.StartObject();
    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(QString::number((int)key)) & (int)value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<TaskStep, TaskStepStatus>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        int status;
        self.Member(key) & status;

        auto taskStep = TaskStep(key.toInt());
        data[taskStep] = TaskStepStatus(status);
    }
    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<TaskStep, TaskStepStatus>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<TaskStep, TaskStepStatus>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<TaskStep, TaskStepStatus>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QMap<TaskStep, TaskStepStatus>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const CurrentRunningTaskPlanData& data)
{
    self.StartObject();

    self.Member("TaskPlanData") & data.taskPlanData;
    self.Member("TaskStepStatusMap") & data.taskStepStatusMap;
    self.Member("Msg") & data.msg;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, CurrentRunningTaskPlanData& data)
{
    self.StartObject();

    self.Member("TaskPlanData") & data.taskPlanData;
    self.Member("TaskStepStatusMap") & data.taskStepStatusMap;
    self.Member("Msg") & data.msg;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, CurrentRunningTaskPlanData& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const CurrentRunningTaskPlanData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, CurrentRunningTaskPlanData& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const CurrentRunningTaskPlanData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
