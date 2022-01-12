#ifndef TASKPLANMESSAGEDEFINE_H
#define TASKPLANMESSAGEDEFINE_H

#include "PlanRunMessageDefine.h"
#include "SystemWorkMode.h"

#include <QColor>
#include <QDateTime>
#include <QMap>
#include <QObject>

#include "GlobalDefine.h"

// 任务计划类型
enum class TaskPlanType
{
    Unknown = 0,          // 未知
    MeasureControl,       // 测控任务
    DataTrans,            // 数传任务
    MeasureCtrlDataTrasn  // 测控+数传任务
};

// 任务计划来源
enum class TaskPlanSource
{
    Unknown = 0,  // 未知
    Local,        // 本地 通过界面创建的计划
    Remote        // 远程 中心创建的计划
};

// 任务计划状态
enum class TaskPlanStatus
{
    NoStart = 0,  // 未开始
    Running,      // 运行中
    TimeOut,      // 超时
    Warning,      // 警告
    Finish,       // 已完成
    Error,        // 错误
    Exception,    // 异常
    CTNoStart,    // 冲突未执行
};

// 标校状态
enum class TaskCalibration
{
    Unknown = -1,         // 未知
    NoCalibration = 0,    // 不标校
    CaliBrationOnce = 1,  // 标校
};

// 任务计划流程的步骤
enum class TaskStep
{
    UNKONW = 0,
    START,
    TASK_PREPARATION,     // 任务准备
    ZERO_CORRECTION,      // 校零处理
    TASK_START,           // 任务开始
    TURN_TOWAITINGPOINT,  // 转等待点
    CALIBRATION,          // 校相处理
    CAPTURE_TRACKLING,    // 捕获跟踪
    GO_UP,                // 发上行
    TRACK_END,            // 跟踪结束
    POST_PROCESSING,      // 事后处理
};

// 任务计划流程每个步骤的状态
enum class TaskStepStatus
{
    NoStart = 0,  // 未开始
    Start,        // 开始
    Running,      // 运行中
    Finish,       // 完成
    Error,        // 错误
    Exception,    // 异常
    Continue      // 跳过
};

// 数传优先级
enum DataTransPriority
{
    Urgent = 1,  // 加急传输
    Priority,    // 优先顺序
    Order,       // 顺序传输
};

// 数传模式
enum class DataTransMode
{
    Unknown = 0,    // 未知
    RealTime,       // 实时数传
    Afterwards,     // 事后数传
    OnlyRecv,       // 只接收
    FirstRealTime,  // 首次事后
};

// 是否共载波 4424需要区分一体化+扩二是否共载波
enum class CarrierType
{
    Unknown = 0,  // 无
    GZB,          // 共载波
    BGZB          // 不共载波
};

enum class CenterProtocol
{
    PDXP = 1,
    FEP = 2,
    HDR = 3,
};

struct AllModeExist
{
    int sttcMode = 0;
    int skuo2Mode = 0;
    int kakuo2Mode = 0;
    int sktMode = 0;
    int xdsMode = 0;
    int kadsMode = 0;
    int kagsMode = 0;
};

// 链路
struct DeviceWorkTaskTargetLink
{
    int working_point_frequency{ 0 };                           // 工作点频
    SystemWorkMode m_work_system{ SystemWorkMode::NotDefine };  // 工作模式
    QString m_codeGroup_yc;                                     // 遥测码组
    QString m_codeGroup_yk;                                     // 遥控码组
    QString m_codeGroup_sxcj;                                   // 上行测距
    QString m_codeGroup_xxcj;                                   // 下行测距
    bool m_taskType_yc{ false };                                // 任务类型 遥测
    bool m_taskType_yk{ false };                                // 任务类型 遥控
    bool m_taskType_cg{ false };                                // 任务类型 测轨
    bool m_taskType_sc{ false };                                // 任务类型 数传
    QString m_baseband_number;                                  // 基带编号
    QString m_linkVal;                                          // 极化方式
    CarrierType m_carrierType{ CarrierType::Unknown };          // 是否共载波
};

#define TimeZero QString("00000000000000")
#define TimeF    QString("FFFFFFFFFFFFFF")
// 目标
struct DeviceWorkTaskTarget
{
    QString m_task_code;                  // 任务代号
    QString m_dt_transNum;                // 跟踪接收计划号
    int m_circle_no{ 0 };                 // 圈号
    QDateTime m_task_ready_start_time;    // 任务准备开始时间
    QDateTime m_task_start_time;          // 任务开始时间
    QDateTime m_track_start_time;         // 跟踪起始时间
    QString m_remote_start_time;          // 遥控开始时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_remote_control_end_time;    // 遥控结束时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_on_uplink_signal_time;      // 开上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_turn_off_line_signal_time;  // 关上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QDateTime m_dt_starttime;             // 数传开始时间        4413新增
    QDateTime m_dt_endtime;               // 数传结束时间        4413新增
    QDateTime m_track_end_time;           // 跟踪结束时间
    QDateTime m_task_end_time;            // 任务结束时间
    QString m_master_control_center;      // 主控中心
    QString m_backup_center;              // 备份中心
    TaskPlanType m_planType{ TaskPlanType::Unknown };  // 计划类型 测控计划  数传计划 测控+数传计划
    int m_link_number{ -1 };                           // 链路数量
    QMap<int, DeviceWorkTaskTargetLink> m_linkMap;     // key : 链路几，后面是具体的那个链路的信息
};

struct DeviceWorkTask
{
    QString m_uuid;                                             // 任务唯一ID
    QString m_create_userid;                                    // 任务的创建者ID 默认的是fep
    QDateTime m_createTime;                                     // 计划创建时间
    QDateTime m_preStartTime;                                   // 准备开始时间
    QDateTime m_startTime;                                      // 任务开始时间
    QDateTime m_endTime;                                        // 计划结束时间
    QString m_plan_serial_number;                               // 计划流水号
    QString m_work_unit;                                        // 工作单位
    QString m_equipment_no;                                     // 设备编号
    QString m_scheduling_mode;                                  // 调度模式
    int m_task_total_num{ 0 };                                  // 任务总数
                                                                //
    TaskCalibration m_is_zero{ TaskCalibration::Unknown };      // 是否校零
    TaskCalibration m_calibration{ TaskCalibration::Unknown };  // 是否校相
                                                                //
    QString m_currentTaskName;                                  // 解析计划的时候是哪一个计划 如任务1、任务2、任务3
    QString m_working_mode;                                     // 工作方式 {"SZ","LT","HL","BJ","ZY"};
    int m_target_number{ 0 };                                   // 目标数量
    QString m_lord_with_target;                                 // 主跟目标
    QString m_tracking_order;                                   // 跟踪顺序
    TaskPlanStatus m_task_status{ TaskPlanStatus::NoStart };    // 任务计划状态
    TaskPlanSource m_source{ TaskPlanSource::Unknown };         // 任务计划来源

    QMap<int, DeviceWorkTaskTarget> m_totalTargetMap;  // key : 目标几，后面是具体的那个目标的信息

    // 获取主跟目标
    bool getMainTarget(DeviceWorkTaskTarget& mainTarget) const
    {
        for (auto& target : m_totalTargetMap)
        {
            if (target.m_task_code == m_lord_with_target)
            {
                mainTarget = target;
                return true;
            }
        }
        return false;
    }

    // 获取主跟目标工作模式
    QList<SystemWorkMode> getMainTargetWorkMode() const
    {
        DeviceWorkTaskTarget mainTarget;
        if (!getMainTarget(mainTarget))
        {
            return {};
        }

        QList<SystemWorkMode> list;
        for (auto link : mainTarget.m_linkMap)
        {
            list << link.m_work_system;
        }

        return list;
    }
};

Q_DECLARE_METATYPE(DeviceWorkTask)

struct DataTranWorkTask
{
    QString m_uuid;                                               // 数传任务唯一ID
    QString m_create_userid;                                      // 任务的创建者ID 默认的是fep
    QDateTime m_createTime;                                       // 计划创建时间
    QDateTime m_task_start_time;                                  // 数传任务开始时间
    QDateTime m_task_end_time;                                    // 数传任务结束时间
    QDateTime m_transport_time;                                   // 事后传输时间
    QDateTime m_task_commit_time;                                 // 计划提交时间 20200725 wangpeng 新加暂时保留
    QString m_plan_serial_number;                                 // 计划流水号
    QString m_work_unit;                                          // 工作单位
    QString m_equipment_no;                                       // 设备编号
    int m_task_total_num{ -1 };                                   // 任务总数
                                                                  //
    TaskPlanStatus m_task_status{ TaskPlanStatus::NoStart };      // 任务计划状态
    DataTransMode m_working_mode{ DataTransMode::Unknown };       // 工作方式 //SS实时 ,SH事后 ,SCSH 首次实时的事后
    QString m_task_code;                                          // 任务代号
    QString m_dt_transNum;                                        // 跟踪接收计划号           4413 新增
    QString m_channelNum;                                         // 通道号                  4413新增
    QString m_datatrans_center;                                   // 数传中心
    CenterProtocol m_center_protocol{ CenterProtocol::PDXP };     // 数传中心协议
    QString m_datatrans_center_id;                                // 数传中心ID
    DataTransPriority m_priority{ DataTransPriority::Priority };  // 数传优先级
    QString m_transport_protocols;                                // 传输协议
    int m_ring_num{ -1 };                                         // 圈次内编号
    TaskPlanSource m_source{ TaskPlanSource::Unknown };           // 任务计划来源
    int m_belongTarget{ -1 };                                     // 该数传任务属于哪个目标
};

Q_DECLARE_METATYPE(DataTranWorkTask)

// 设备文件计划文件信息
struct DeviceWorkTaskFileInfo
{
    int m_version_number{ 0 };       // 版本号
    QString m_object_identifying;    // 对象标识
    QString m_source_identifying;    // 信源标识
    QString m_mode_identifying;      // 模式标识
    QString m_msg_type_identifying;  // 信息类别标识
    QString m_date_identifying;      // 日期标识
    QString m_moment_identifying;    // 时刻标识
    QString m_number;                // 编号
};

using DeviceWorkTaskList = QList<DeviceWorkTask>;
using DataTranWorkTaskList = QList<DataTranWorkTask>;

Q_DECLARE_METATYPE(DeviceWorkTaskList)
Q_DECLARE_METATYPE(DataTranWorkTaskList)

struct TaskPlanData
{
    DeviceWorkTask deviceWorkTask;
    DataTranWorkTaskList dataTransWorkTasks;
};

struct DeviceWorkTaskQuery
{
    QDate beginDate;
    QDate endDate;
};

struct DeviceWorkTaskConditionQuery
{
    int taskStatus{ 0 };
    QDateTime beginDate;
    QDateTime endDate;
    int totalCount{ 0 };
    int currentPage{ 0 };
    int pageSize{ 0 };
};

struct DeviceWorkTaskConditionQueryACK
{
    int totalCount{ 0 };
    int currentPage{ 0 };
    DeviceWorkTaskList deviceWorkTasks;
};
/* 状态更新 */
struct DeviceWorkTaskUpdate
{
    QString uuid;
    TaskPlanStatus planStatus{ TaskPlanStatus::NoStart };
};
/* 删除 */
struct DeviceWorkTaskDelete
{
    QString uuid;
};
/* 查看 */
struct DeviceWorkTaskView
{
    QString id;
    QString sn;
};

// 当前运行的任务计划信息推送数据
struct CurrentRunningTaskPlanData
{
    TaskPlanData taskPlanData;                          // 当前运行的任务
    QMap<TaskStep, TaskStepStatus> taskStepStatusMap;   // 任务计划每个步骤的状态
    ManualMessage msg;  // 任务流程控制的信息，用户要求把手动下发的参数宏信息也显示在主界面的任务计划上
};

class TaskPlanHelper
{
public:
    static QString carrierTypeToString(const CarrierType& type)
    {
        switch (type)
        {
        case CarrierType::GZB: return QString("共载波");
        case CarrierType::BGZB: return QString("不共载波");
        case CarrierType::Unknown: return QString("无");
        }
        return QString("无");
    }

    static QString taskTypeToString(const TaskPlanType& type)
    {
        switch (type)
        {
        case TaskPlanType::MeasureControl: return QString("测控任务");
        case TaskPlanType::DataTrans: return QString("数传任务");
        case TaskPlanType::MeasureCtrlDataTrasn: return QString("测控+数传任务");
        case TaskPlanType::Unknown: break;
        }
        return QString("未知");
    }

    static QString taskSourceToString(const TaskPlanSource& source)
    {
        switch (source)
        {
        case TaskPlanSource::Local: return QString("本地");
        case TaskPlanSource::Remote: return QString("远程");
        case TaskPlanSource::Unknown: break;
        }
        return QString("未知");
    }

    static QString taskStatusToString(const TaskPlanStatus& status)
    {
        switch (status)
        {
        case TaskPlanStatus::NoStart: return QString("未开始");
        case TaskPlanStatus::Running: return QString("正在运行");
        case TaskPlanStatus::TimeOut: return QString("超时");
        case TaskPlanStatus::Warning: return QString("警告");
        case TaskPlanStatus::Finish: return QString("完成");
        case TaskPlanStatus::Error: return QString("错误");
        case TaskPlanStatus::Exception: return QString("异常");
        case TaskPlanStatus::CTNoStart: return QString("冲突未执行");
        }
        return QString("未知");
    }

    static QColor taskStatusToColor(const TaskPlanStatus& status)
    {
        switch (status)
        {
        case TaskPlanStatus::NoStart: return QColor(Qt::black);
        case TaskPlanStatus::Running: return QColor(Qt::green);
        case TaskPlanStatus::TimeOut: return QColor(Qt::red);
        case TaskPlanStatus::Warning: return QColor(Qt::yellow);
        case TaskPlanStatus::Finish: return QColor(Qt::green);
        case TaskPlanStatus::Error:
        case TaskPlanStatus::Exception:
        case TaskPlanStatus::CTNoStart: return QColor(Qt::red);
        }
        return QColor(Qt::black);
    }

    static QString dataTransPriorityToString(const DataTransPriority& priority)
    {
        switch (priority)
        {
        case DataTransPriority::Urgent: return QString("加急传输");
        case DataTransPriority::Priority: return QString("优先顺序");
        case DataTransPriority::Order: return QString("顺序传输");
        }
        return QString("未知");
    }
    static QString dataTransModeToString(const DataTransMode& mode)
    {
        switch (mode)
        {
        case DataTransMode::RealTime: return QString("实时数传");
        case DataTransMode::Afterwards: return QString("事后数传");
        case DataTransMode::OnlyRecv: return QString("只接收");
        case DataTransMode::FirstRealTime: return QString("首次事后");
        case DataTransMode::Unknown: break;
        }
        return QString("未知");
    }
    static DataTransMode KeyStringToDataTransMode(const QString& key)
    {
        if (key == "SS")
        {
            return DataTransMode::RealTime;
        }
        else if (key == "SH")
        {
            return DataTransMode::Afterwards;
        }
        else if (key == "SCSH")
        {
            return DataTransMode::OnlyRecv;
        }
        return DataTransMode::Unknown;
    }
    static QString planWorkModeToDesc(const QString& mode)
    {
        if (mode == "SZ")
            return "实战实时";
        else if (mode == "LT")
            return "联调实时";
        else if (mode == "HL")
            return "全区合练";
        else
            return mode;
    }

    static QString centerProtocolToDesc(CenterProtocol centerProtocol)
    {
        if (centerProtocol == CenterProtocol::PDXP)
            return "PDXP";
        else if (centerProtocol == CenterProtocol::FEP)
            return "FEP";
        else if (centerProtocol == CenterProtocol::HDR)
            return "HDR";
        else
            return "Unknown";
    }
};

#endif  // TASKPLANMESSAGEDEFINE_H
