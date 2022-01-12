#ifndef TASKRUNCOMMONHELPER_H
#define TASKRUNCOMMONHELPER_H
#include "ConfigMacroMessageDefine.h"
#include "PhaseCalibrationDefine.h"
#include "PlanRunMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"
#include "SatelliteManagementDefine.h"
#include "SystemWorkMode.h"
#include "TaskPlanMessageDefine.h"
#include "Utility.h"
#include <QMap>
#include <QStringList>

struct TaskTimeEntity
{
    quint64 m_equipNum = 0;               // 设备组合号
    quint64 m_backequipNum = 0;           // 备用的设备组合号
    QString m_taskCode;                   // 任务代号
    quint64 m_taskBz;                     // 任务标识
    int m_dpNum = 0;                      // 点频几
    double m_upFrequency = 0;             // 上行频率
    double m_downFrequency = 0;           // 下行频率
    QDateTime m_task_ready_start_time;    //任务准备开始时间
    QDateTime m_task_start_time;          //任务开始时间
    QDateTime m_trace_start_time;         //跟踪起始时间
    QString m_remote_start_time;          //遥控开始时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_remote_control_end_time;    //遥控结束时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_on_uplink_signal_time;      //开上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_turn_off_line_signal_time;  //关上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QDateTime m_dt_starttime;             //数传开始时间        4413新增
    QDateTime m_dt_endtime;               //数传结束时间        4413新增
    QDateTime m_tracking_end_time;        //跟踪结束时间
    QDateTime m_task_end_time;            //任务结束时间
    int m_circle_no = 0;                  // 圈号
    QString m_planSerialNumber;           // 计划流水号
    QString m_dt_transNum;                // 是一个9位的固定数,跟踪接收计划号
    bool isMaster = false;                // 判断是否是主跟踪目标
    bool m_taskType_yc = 0;               // 任务类型 遥测
    bool m_taskType_yk = 0;               // 任务类型 遥控
    bool m_taskType_cg = 0;               // 任务类型 测轨
    bool m_taskType_sc = 0;               // 任务类型 数传
    QString m_taskxuanxiang;              // 任务旋向    LHCP 左旋 RHCP 右旋  HLP VLP左右旋同时
};
struct ModeExist
{
    bool sttcMode = false;
    bool skuo2Mode = false;
    bool kakuo2Mode = false;
    bool sktMode = false;
    bool xdsMode = false;
    bool kadsMode = false;
    bool kagsMode = false;
};
struct TaskTimeList
{
    ModeExist modeExist;        /* 当前任务那些模式存在那些不存在  !m_taskTimeList.m_dataMap.contains 写的太长了 */
    bool existCKMode{ false };  // 是否存在测控模式
    QMap<int, QList<TaskTimeEntity>> m_dataMap;  // 1-6 代表STTC SKuo2 Kakuo2 xds kads kags
    // QMap<int, QString> m_masterTaskCodeMap;      //主跟任务代号
    // QMap<int, int> m_masterTaskBzMap;      //主跟任务标识
    QString masterSataTaskCode;  //主跟任务代号
    QString masterSataTaskBz;    //主跟任务标识
    // QMap<int, int> m_currentXuanXiangMap;  //工作模式对应的旋向
    // QMap<DeviceID, QList<int>> m_gzJWCdeviceMap;      //跟踪基带对应的工作模式
    // QMap<int, double> m_upFrequencyMap;               //上行频率
    // QMap<int, double> m_downFrequencyMap;             //下行频率
    // QMap<int, QList<double>> m_upFrequencyListMap;    //多目标的上行点频
    // QMap<int, QList<double>> m_downFrequencyListMap;  //多目标的下行点频
    // QMap<QString, QString> m_taskCode2SCCenterMap;  //任务代号对应的数传中心

    // 1:S 2:Ka遥测 3:Ka低速数传 4:Ka高速数传 5:S+Ka遥测 6:S+Ka低速数传 7:S+Ka高速数传 8:S+Ka遥测+Ka低速数传 9:S+Ka遥测+Ka高速数传
    SatelliteTrackingMode trackingMode = SatelliteTrackingMode::TrackUnknown;  //跟踪方式选择，卫星
    // 跟踪用哪个点频的数据 这里只要计划解析成功就是1
    // int trackingPointFreqNo{ -1 };

    // ConfigMacroData configMacroData;
    ManualMessage manualMessage;          // 参数宏配置宏这边用完后给校零用
    DeviceID onlineACU;                   // 当前在线ACU设备ID
    ParamMacroData masterParamMacroData;  // 主跟的参数宏
    TaskPlanData taskPlanData;            //当前的任务计划

    // QMap<int, DeviceID> m_ckDeviceMap;  //测控模式下的测控基带信息   key代表工作模式
    // QMap<int, DeviceID> m_dsDeviceMap;  //低速基带信息
    // QMap<int, DeviceID> m_gsDeviceMap;  //高速基带信息

    // QMap<int, DeviceID> m_ckQdDeviceMap;      //测控前端信息
    // QMap<int, DeviceID> m_ckQdDeviceBackMap;  //测控前端备份使用，双点频使用
    // QMap<int, DeviceID> m_gzDeviceMap;  //跟踪基带信息
    // QMap<int, DeviceID> m_gzQdDeviceMap;      //跟踪前端信息

    // QMap<int, DeviceID> m_hpaDeviceMap;  //功放的信息
    // QMap<int, DeviceID> m_satDeviceMap;  //存储转发

    // QMap<DeviceID, QList<QMap<QString, QVariant>>> m_cczfCmdMap;  //记录一个任务计划下发时所有的存储转发计划命令
    // QMap<DeviceID, QList<QMap<QString, QVariant>>> m_dteCmdMap;   //记录一个任务计划下发时所有的dte命令

    // int m_kadsChannel{ -1 }; /* 存转的Ka单点频低速通道 */
    // int m_xdsChannel{ -1 };  /* 存转的x单点频低速通道 */

    // QString m_working_mode;  // SZ:实战，LT:联调,ZY:占用，BJ:标校

    // bool haveUpdate = false;  //用于标识ZY的任务时间变化了 流程中做对应处理
    // TaskConfigInfo commonTaskConfig;  //用于自动化运行的公共配置

    QDateTime m_task_ready_start_time;    //任务准备开始时间
    QDateTime m_task_start_time;          //任务开始时间
    QDateTime m_track_start_time;         //跟踪起始时间
    QString m_remote_start_time;          //遥控开始时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_remote_control_end_time;    //遥控结束时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_on_uplink_signal_time;      //开上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_turn_off_line_signal_time;  //关上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QDateTime m_dt_starttime;             //数传开始时间        4413新增
    QDateTime m_dt_endtime;               //数传结束时间        4413新增
    QDateTime m_track_end_time;           //跟踪结束时间
    QDateTime m_task_end_time;            //任务结束时间

    //根据角误差给实际的信号，比如当前选择的任务工作模式是标准，S扩2，Ka低速，Ka高速，但是我跟踪的旋向只有标准TTC，这时候只需要将一个信号的角误差给ACU
    // QMap<int, bool> m_jwcSendMap;  //送数的角误差信号
    bool m_is_zero = false;      //是否校零
    bool m_calibration = false;  //是否校相
};
class TaskRunCommonHelper
{
public:
    static QMap<QString, int> m_paramMap;

    static void getSatelliteXuanxiang(SatelliteManagementData satellite, SystemWorkMode workMode, int dpNum, QString& xuanxiang);

    /* 工作计划解析 */
    static Optional<TaskTimeList> getDeviceWorkTaskInfo(const TaskPlanData& taskPlanData);

    static QString responceInfo(int logLevel);

private:
};

#endif  // TASKCOMMONHELPER_H
