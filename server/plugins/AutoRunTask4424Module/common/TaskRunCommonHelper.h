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
struct TaskConfigInfo
{
    double zeroWaitLockTime = 3;     //校零等待锁定时间
    double zeroFW = 5;               //校零转等待点的方位
    double zeroFY = 5;               //校零转等待点的俯仰
    int biaoxiaoTime = 40;           //校相时间
    int zeroTime = 40;               //校零时间,倒计时
    double STTCCorrectValue = 30;    // TCC修正值
    double SKUO2CorrectValue = 30;   // S扩频修正值
    double KAKUO2CorrectValue = 30;  // Ka扩频修正值

    PhaseCalibrationCorrMeth phaseCorrMeth = PhaseCalibrationCorrMeth::Satellite;  // 校相方式 2为当前 3是对星

    int sCrossLowLimit = 5;                                                           /* 交叉耦合下限值(分母) */
    int sStandSensit = 500;                                                           /* 定向灵敏度标准值(mv) */
    int sDirectToler = 50;                                                            /* 定向灵敏度允差(mv) */
    PhaseCalibrationFollowCheck sFollowCheck = PhaseCalibrationFollowCheck::NotCheck; /* 自跟踪检查 */

    int kaCrossLowLimit = 5;                                                           /* 交叉耦合下限值(分母) */
    int kaStandSensit = 4800;                                                          /* 定向灵敏度标准值(mv) */
    int kaDirectToler = 400;                                                           /* 定向灵敏度允差(mv) */
    PhaseCalibrationFollowCheck kaFollowCheck = PhaseCalibrationFollowCheck::NotCheck; /* 自跟踪检查 */

    int kadtCrossLowLimit = 5;                                                           /* 交叉耦合下限值(分母) */
    int kadtStandSensit = 4800;                                                          /* 定向灵敏度标准值(mv) */
    int kadtDirectToler = 400;                                                           /* 定向灵敏度允差(mv) */
    PhaseCalibrationFollowCheck kadtFollowCheck = PhaseCalibrationFollowCheck::NotCheck; /* 自跟踪检查 */
};
struct TaskTimeEntity
{
    quint64 m_equipNum = 0;               // 设备组合号
    quint64 m_backequipNum = 0;           // 备用的设备组合号
    QString m_taskCode;                   // 任务代号
    quint64 m_taskBz;                     // 任务标识
    int m_dpNum;                          // 点频几
    double m_upFrequency = 0;             // 上行频率
    double m_downFrequency = 0;           // 下行频率
    QDateTime m_task_ready_start_time;    // 任务准备开始时间
    QDateTime m_task_start_time;          // 任务开始时间
    QDateTime m_trace_start_time;         // 跟踪起始时间
    QString m_remote_start_time;          // 遥控开始时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_remote_control_end_time;    // 遥控结束时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_on_uplink_signal_time;      // 开上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QString m_turn_off_line_signal_time;  // 关上行信号时间 这个时间全0是不发 全F是自动  这个有特殊意义不能为DataTime
    QDateTime m_dt_starttime;             // 数传开始时间        4413新增
    QDateTime m_dt_endtime;               // 数传结束时间        4413新增
    QDateTime m_tracking_end_time;        // 跟踪结束时间
    QDateTime m_task_end_time;            // 任务结束时间
    int m_circle_no;                      // 圈号
    QString m_planSerialNumber;           // 计划流水号
    QString m_dt_transNum;                // 是一个9位的固定数,跟踪接收计划号
    bool isMaster = true;                 // 判断是否是主跟踪目标
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
    ModeExist modeExist; /* 当前任务那些模式存在那些不存在  !m_taskTimeList.m_dataMap.contains 写的太长了 */
    QMap<int, QList<TaskTimeEntity>> m_dataMap;  // 1-6 代表STTC SKuo2 Kakuo2 xds kads kags
    QString masterTaskCode;                      // 主跟任务代号
    QString masterTaskBz;                        // 主跟任务标识
    QMap<int, int> m_currentXuanXiangMap;        // 工作模式对应的旋向
    // QMap<DeviceID, QList<int>> m_gzJWCdeviceMap;      //跟踪基带对应的工作模式
    QMap<int, double> m_upFrequencyMap;               //上行频率
    QMap<int, double> m_downFrequencyMap;             //下行频率
    QMap<int, QList<double>> m_upFrequencyListMap;    //多目标的上行点频
    QMap<int, QList<double>> m_downFrequencyListMap;  //多目标的下行点频
    QMap<QString, QString> m_taskCode2SCCenterMap;    //任务代号对应的数传中心

    // 1:S 2:Ka遥测 3:Ka低速数传 4:Ka高速数传 5:S+Ka遥测 6:S+Ka低速数传 7:S+Ka高速数传 8:S+Ka遥测+Ka低速数传 9:S+Ka遥测+Ka高速数传
    SatelliteTrackingMode trackingMode = SatelliteTrackingMode::TrackUnknown;  //跟踪方式选择，卫星
    // 跟踪用哪个点频的数据 这里只要计划解析成功就是1
    int trackingPointFreqNo{ -1 };

    ManualMessage manualMessage;          // 参数宏配置宏这边用完后给校零用
    DeviceID onlineACU;                   // 当前在线ACU设备ID
    ParamMacroData masterParamMacroData;  // 主跟的参数宏
    TaskPlanData taskPlanData;            //当前的任务计划

    QMap<int, DeviceID> m_ckDeviceMap;  //测控模式下的测控基带信息   key代表工作模式
    QMap<int, DeviceID> m_dsDeviceMap;  //低速基带信息
    QMap<int, DeviceID> m_gsDeviceMap;  //高速基带信息

    QMap<int, DeviceID> m_ckQdDeviceMap;      //测控前端信息
    QMap<int, DeviceID> m_ckQdDeviceBackMap;  //测控前端备份使用，双点频使用
    QMap<int, DeviceID> m_gzDeviceMap;        //跟踪基带信息
    QMap<int, DeviceID> m_gzQdDeviceMap;      //跟踪前端信息

    QMap<DeviceID, QList<QMap<QString, QVariant>>> m_cczfCmdMap;  //记录一个任务计划下发时所有的存储转发计划命令
    QMap<DeviceID, QList<QMap<QString, QVariant>>> m_dteCmdMap;   //记录一个任务计划下发时所有的dte命令

    QMap<int, DeviceID> m_hpaDeviceMap;  //功放的信息

    QMap<int, DeviceID> m_satDeviceMap;  //存储转发
    int m_kadsChannel{ -1 };             /* 存转的Ka单点频低速通道 */
    int m_xdsChannel{ -1 };              /* 存转的x单点频低速通道 */

    QString m_working_mode;  // SZ:实战，LT:联调,ZY:占用，BJ:标校

    bool haveUpdate = false;  //用于标识ZY的任务时间变化了 流程中做对应处理

    TaskConfigInfo commonTaskConfig;  //用于自动化运行的公共配置

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

    DeviceWorkTask m_deviceWorkTask;  //当前的设备工作计划
    //根据角误差给实际的信号，比如当前选择的任务工作模式是标准，S扩2，Ka低速，Ka高速，但是我跟踪的旋向只有标准TTC，这时候只需要将一个信号的角误差给ACU
    QMap<int, bool> m_jwcSendMap;  //送数的角误差信号
    bool m_is_zero = false;        //是否校零
    bool m_calibration = false;    //是否校相

    quint32 txUACAddr{ 0 }; /* 天线UAC地址/站UAC地址 */

    bool isEqual(TaskTimeList pTaskTimeList)
    {
        if (m_working_mode != pTaskTimeList.m_working_mode)
        {
            return false;
        }
        if (m_dataMap.size() != pTaskTimeList.m_dataMap.size())
        {
            return false;
        }
        for (auto i = 0; i < m_dataMap.keys().size(); i++)
        {
            if (m_dataMap.keys()[i] != pTaskTimeList.m_dataMap.keys()[i])
            {
                return false;
            }
        }
        for (auto key : m_dataMap.keys())
        {
            auto list = m_dataMap[key];
            auto list2 = pTaskTimeList.m_dataMap[key];
            if (list.size() != list2.size())
            {
                return false;
            }
            std::sort(list.begin(), list.end(), [=](TaskTimeEntity a, TaskTimeEntity b) { return a.m_taskCode < b.m_taskCode; });
            std::sort(list2.begin(), list2.end(), [=](TaskTimeEntity a, TaskTimeEntity b) { return a.m_taskCode < b.m_taskCode; });
            for (auto i = 0; i < list.size(); i++)
            {
                if (list[i].m_taskCode != list2[i].m_taskCode)
                {
                    return false;
                }
                if (list[i].m_dpNum != list2[i].m_dpNum)
                {
                    return false;
                }
                if (list[i].isMaster != list2[i].isMaster)
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool updateDataTime(TaskTimeList pTaskTimeList)
    {
        for (auto key : m_dataMap.keys())
        {
            auto& list = m_dataMap[key];
            auto& list2 = pTaskTimeList.m_dataMap[key];

            std::sort(list.begin(), list.end(), [=](TaskTimeEntity a, TaskTimeEntity b) { return a.m_taskCode < b.m_taskCode; });
            std::sort(list2.begin(), list2.end(), [=](TaskTimeEntity a, TaskTimeEntity b) { return a.m_taskCode < b.m_taskCode; });
            for (auto i = 0; i < list.size(); i++)
            {
                if (list[i].m_taskCode == list2[i].m_taskCode && list[i].m_dpNum == list2[i].m_dpNum)
                {
                    list[i].m_remote_start_time = list2[i].m_remote_start_time;
                    list[i].m_remote_control_end_time = list2[i].m_remote_control_end_time;
                    list[i].m_on_uplink_signal_time = list2[i].m_on_uplink_signal_time;
                    list[i].m_turn_off_line_signal_time = list2[i].m_turn_off_line_signal_time;
                    list[i].m_tracking_end_time = list2[i].m_tracking_end_time;
                    list[i].m_dt_endtime = list2[i].m_dt_endtime;
                    list[i].m_task_end_time = list2[i].m_task_end_time;
                }
            }
        }
        return true;
    }
    QStringList getGSDeviceNameList()
    {
        if (m_gsDeviceMap.contains(KaGS))
        {
            return QStringList() << QString("HDD%1").arg(m_gsDeviceMap[KaGS].extenID) + "01"
                                 << QString("HDD%1").arg(m_gsDeviceMap[KaGS].extenID) + "02";
        }
        return QStringList();
    }
    QStringList getGSBackDeviceName()
    {
        if (m_gsDeviceMap.contains(KaGS))
        {
            auto deviceBackID = m_gsDeviceMap[KaGS];
            if (deviceBackID.extenID == 1)
                deviceBackID.extenID = 2;
            else
                deviceBackID.extenID = 1;
            return QStringList() << QString("HDD%1").arg(deviceBackID.extenID) + "01" << QString("HDD%1").arg(deviceBackID.extenID) + "02";
        }
        return QStringList();
    }
    QString getXDSDeviceName() { return m_dsDeviceMap.contains(XDS) ? (QString("LDD%1").arg(m_dsDeviceMap[XDS].extenID) + "02") : QString(); }
    QStringList getXDSDeviceNameList()
    {
        if (m_dsDeviceMap.contains(XDS))
        {
            if (m_dsDeviceMap[XDS].extenID == 1)
            {
                return QStringList() << QString("LDD%1").arg(m_dsDeviceMap[XDS].extenID) + "02" << QString("LDD%1").arg(2) + "02";
            }
            else
            {
                return QStringList() << QString("LDD%1").arg(m_dsDeviceMap[XDS].extenID) + "02" << QString("LDD%1").arg(1) + "02";
            }
        }
        return QStringList();
    }
    QString getKaDSDeviceName() { return m_dsDeviceMap.contains(KaDS) ? (QString("LDD%1").arg(m_dsDeviceMap[KaDS].extenID) + "01") : QString(); }
    QStringList getKaDSDeviceNameList()
    {
        if (m_dsDeviceMap[KaDS].extenID == 1)
        {
            return QStringList() << QString("LDD%1").arg(m_dsDeviceMap[KaDS].extenID) + "01" << QString("LDD%1").arg(2) + "01";
        }
        else
        {
            return QStringList() << QString("LDD%1").arg(m_dsDeviceMap[KaDS].extenID) + "01" << QString("LDD%1").arg(1) + "01";
        }
        return QStringList();
    }
};

class TaskRunCommonHelper
{
public:
    /* 获取站测控UAC地址 */
    static Optional<quint32> getStationCKUAC();

    static void getSatelliteXuanxiang(SatelliteManagementData satellite, SystemWorkMode workMode, int dpNum, QString& xuanxiang);

    /* 跟踪模式转换为对应的系统工作模式 */
    static SystemWorkMode trackingToSystemWorkMode(SatelliteTrackingMode mode);
    /* 获取ACU的系统工作方式 */
    static Optional<int> getACUSystemWorkMode(SatelliteTrackingMode mode);
    /* 工作计划解析 */
    static Optional<TaskTimeList> getDeviceWorkTaskInfo(const DeviceWorkTask& deviceWorkTask);

    /* 两种跟踪的转换 */
    static ACUPhaseCalibrationFreqBand trackingModeToAcuFreqBandStatus(SatelliteTrackingMode mode);

private:
    // 当前下发的任务模式和设置的跟踪模式对应不上时需要处理一下
    static OptionalNotValue checkTrackingMode(TaskTimeList& taskTimeList);
    // 检查跟踪点频
    static OptionalNotValue checkTrackingPointFreq(TaskTimeList& taskTimeList);
};

#endif  // TASKCOMMONHELPER_H
