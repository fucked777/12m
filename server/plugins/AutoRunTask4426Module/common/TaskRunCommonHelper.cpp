#include "TaskRunCommonHelper.h"

#include "BaseHandler.h"
#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PhaseCalibrationSerialize.h"
#include "RedisHelper.h"
#include "StationResManagementSerialize.h"

void TaskRunCommonHelper::getSatelliteXuanxiang(SatelliteManagementData satellite, SystemWorkMode workMode, int dpNum, QString& xuanxiang)
{
    if (xuanxiang == "LHCP" || xuanxiang == "RHCP" || xuanxiang == "HLP" || xuanxiang == "VLP")
    {
        return;
    }
    if (satellite.m_workModeParamMap.contains(workMode))
    {
        SystemOrientation systemOrientation;
        if (satellite.getSReceivPolar(workMode, dpNum, systemOrientation))
        {
            if (systemOrientation == SystemOrientation::LCircumflex)
            {
                xuanxiang = "LHCP";
                return;
            }
            if (systemOrientation == SystemOrientation::RCircumflex)
            {
                xuanxiang = "RHCP";
                return;
            }
            if (systemOrientation == SystemOrientation::LRCircumflex)
            {
                xuanxiang = "HLP";
                return;
            }
        }
    }
    //默认数据是左旋
    xuanxiang = "LHCP";
}

Optional<TaskTimeList> TaskRunCommonHelper::getDeviceWorkTaskInfo(const TaskPlanData& taskPlanData)
{
    using ResType = Optional<TaskTimeList>;

    auto& deviceWorkTask = taskPlanData.deviceWorkTask;
    // 先检查一下
    if (deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "计划目标数据为空计划数据错误");
    }
    if (deviceWorkTask.m_totalTargetMap.size() != deviceWorkTask.m_target_number)
    {
        auto msg = QString("计划的目标数量不匹配 计划目标数量 %1 实际目标数量 %2")
                       .arg(deviceWorkTask.m_target_number)
                       .arg(deviceWorkTask.m_totalTargetMap.size());
        return ResType(ErrorCode::InvalidArgument, msg);
    }
    //获取主跟目标信息
    auto lord_with_target = deviceWorkTask.m_lord_with_target;
    int masterCount = 0;
    for (auto& target : deviceWorkTask.m_totalTargetMap)
    {
        auto eq = (target.m_task_code.toUpper() == lord_with_target.toUpper());
        masterCount += static_cast<int>(eq);
    }
    if (masterCount <= 0)
    {
        auto msg = QString("计划中未找到主跟目标的相关信息:%1").arg(lord_with_target);
        return ResType(ErrorCode::InvalidArgument, msg);
    }

    TaskTimeList taskTimeList;
    //更新设备工作计划
    taskTimeList.taskPlanData = taskPlanData;
    // taskTimeList.m_working_mode = deviceWorkTask.m_working_mode;
    taskTimeList.m_is_zero = deviceWorkTask.m_is_zero == TaskCalibration::CaliBrationOnce;
    taskTimeList.m_calibration = deviceWorkTask.m_calibration == TaskCalibration::CaliBrationOnce;
    taskTimeList.masterSataTaskCode = lord_with_target;  // 主跟任务代号

    //获取每一个目标的任务代号
    for (auto& target : deviceWorkTask.m_totalTargetMap)
    {
        SatelliteManagementData satelliteTemp;
        if (!GlobalData::getSatelliteManagementData(target.m_task_code, satelliteTemp))
        {
            return ResType(ErrorCode::InvalidArgument, "自动化运行无法获取正确的任务代号");
        }
        if (target.m_task_code == lord_with_target)
        {
            //主跟目标对应时间   如果目标数大于1 就使用主根目标的时间做发上行，载波等。
            taskTimeList.trackingMode = satelliteTemp.m_trackingMode;
            taskTimeList.m_task_ready_start_time = target.m_task_ready_start_time;          //任务准备开始时间
            taskTimeList.m_task_start_time = target.m_task_start_time;                      //任务开始时间
            taskTimeList.m_track_start_time = target.m_track_start_time;                    //跟踪起始时间
            taskTimeList.m_remote_start_time = target.m_remote_start_time;                  //遥控开始时间
            taskTimeList.m_remote_control_end_time = target.m_remote_control_end_time;      //遥控结束时间
            taskTimeList.m_on_uplink_signal_time = target.m_on_uplink_signal_time;          //开上行信号时间
            taskTimeList.m_turn_off_line_signal_time = target.m_turn_off_line_signal_time;  //关上行信号时间
            taskTimeList.m_dt_starttime = target.m_dt_starttime;                            //数传开始时间        4413新增
            taskTimeList.m_dt_endtime = target.m_dt_endtime;                                //数传结束时间        4413新增
            taskTimeList.m_track_end_time = target.m_track_end_time;                        //跟踪结束时间
            taskTimeList.m_task_end_time = target.m_task_end_time;                          //任务结束时间
        }
        auto tempTaskCode = target.m_task_code;
        for (auto& link : target.m_linkMap)
        {
            TaskTimeEntity entity;
            entity.isMaster = (tempTaskCode == lord_with_target);
            entity.m_taskCode = tempTaskCode;
            entity.m_dpNum = link.working_point_frequency;
            entity.m_task_ready_start_time = target.m_task_ready_start_time;
            entity.m_task_start_time = target.m_task_start_time;
            entity.m_trace_start_time = target.m_track_start_time;
            entity.m_remote_start_time = target.m_remote_start_time;
            entity.m_remote_control_end_time = target.m_remote_control_end_time;
            entity.m_on_uplink_signal_time = target.m_on_uplink_signal_time;
            entity.m_turn_off_line_signal_time = target.m_turn_off_line_signal_time;
            entity.m_tracking_end_time = target.m_track_end_time;
            entity.m_task_end_time = target.m_task_end_time;
            entity.m_dt_starttime = target.m_dt_starttime;
            entity.m_dt_endtime = target.m_dt_endtime;

            entity.m_dt_transNum = target.m_dt_transNum;                      //处理跟踪接收计划号
            entity.m_planSerialNumber = deviceWorkTask.m_plan_serial_number;  //计划流水号
            entity.m_circle_no = target.m_circle_no;                          //圈次号
            entity.m_taskType_yc = link.m_taskType_yc;
            entity.m_taskType_cg = link.m_taskType_cg;
            entity.m_taskType_yk = link.m_taskType_yk;
            entity.m_taskType_sc = link.m_taskType_sc;

            entity.m_taskxuanxiang = link.m_linkVal;
            // taskTimeList.m_taskCode2SCCenterMap[tempTaskCode] = satelliteTemp.m_digitalCenterID;  // 更新数传中心标识
            int dpNum = link.working_point_frequency;
            auto currentWorkMode = link.m_work_system;
            getSatelliteXuanxiang(satelliteTemp, link.m_work_system, dpNum, entity.m_taskxuanxiang);

            //链路里面需要分测控还是数传，如果链路1是测控就配置测控链路，如果链路2是数传就配置数传链路   任务代号这里泛指的是标准+数传
            //假如链路只有一个，就是单体质工作，判断链路工作模式，如果是测控体制，就是测控任务，如果是数传体制就是数传任务。
            //假如链路有2天，还同时是测控体制，这种情况就是测控双点频，需要配置主用和备用链路。
            //假如是2个目标，单体制，根据使用的工作体制来判断是标准双点频，还是扩频多目标。
            auto rwbz = satelliteTemp.m_satelliteIdentification.toUInt(nullptr, 16);
            entity.m_taskBz = rwbz;  //更新任务标识
            taskTimeList.m_dataMap[currentWorkMode].append(entity);
            if (tempTaskCode == lord_with_target)
            {
                taskTimeList.masterSataTaskBz = satelliteTemp.m_satelliteIdentification;
            }
        }
    }

    if (taskTimeList.masterSataTaskCode.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前任务计划主跟目标为空计划错误");
    }

    //处理S扩二 和 一体化 组合问题
    //暂时不支持这种任务方式

    // TODO 处理Skuo2 Kakuo2单点频 双极化 HLP VLP 双极化
    if (taskTimeList.m_dataMap.contains(Skuo2) && taskTimeList.m_dataMap[Skuo2].size() == 1 &&
        (taskTimeList.m_dataMap[Skuo2][0].m_taskxuanxiang == "HLP" || taskTimeList.m_dataMap[Skuo2][0].m_taskxuanxiang == "VLP"))
    {
        taskTimeList.m_dataMap[Skuo2].append(taskTimeList.m_dataMap[Skuo2][0]);
    }

    //
    if (taskTimeList.m_dataMap.contains(KaKuo2) && taskTimeList.m_dataMap[KaKuo2].size() == 1 &&
        (taskTimeList.m_dataMap[KaKuo2][0].m_taskxuanxiang == "HLP" || taskTimeList.m_dataMap[KaKuo2][0].m_taskxuanxiang == "VLP"))
    {
        taskTimeList.m_dataMap[KaKuo2].append(taskTimeList.m_dataMap[KaKuo2][0]);
    }

    auto& modeExist = taskTimeList.modeExist;
    modeExist.sttcMode = taskTimeList.m_dataMap.contains(STTC);
    modeExist.skuo2Mode = taskTimeList.m_dataMap.contains(Skuo2);
    modeExist.kakuo2Mode = taskTimeList.m_dataMap.contains(KaKuo2);
    modeExist.sktMode = taskTimeList.m_dataMap.contains(SKT);
    modeExist.xdsMode = taskTimeList.m_dataMap.contains(XDS);
    modeExist.kadsMode = taskTimeList.m_dataMap.contains(KaDS);
    modeExist.kagsMode = taskTimeList.m_dataMap.contains(KaGS);

    QSet<SystemWorkMode> ckWorkModeSet;
    for (auto iter = taskTimeList.m_dataMap.begin(); iter != taskTimeList.m_dataMap.end(); ++iter)
    {
        auto workMode = SystemWorkMode(iter.key());
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            ckWorkModeSet << workMode;
        }
    }
    taskTimeList.existCKMode = !ckWorkModeSet.isEmpty();

    return ResType(taskTimeList);
}

QString TaskRunCommonHelper::responceInfo(int logLevel)
{
    if (logLevel == 0)
    {
        //正常接收并执行
        // messageInfo+=("执行成功");
        return "执行成功";
    }
    else if (logLevel == 1)
    {
        //分控拒收
        // messageInfo+=("分控拒收");
        return "分控拒收";
    }
    else if (logLevel == 2)
    {
        //帧格式错误
        return "分控拒收";
    }
    else if (logLevel == 3)
    {
        //被控对象不存在
        return "被控对象不存在";
    }
    else if (logLevel == 4)
    {
        //参数错误
        return "参数错误";
    }
    else if (logLevel == 5)
    {
        //条件不具备
        return "条件不具备";
    }
    else if (logLevel == 6)
    {
        //未知原因错误
        return "未知原因错误";
    }
    return "其他错误";
}
