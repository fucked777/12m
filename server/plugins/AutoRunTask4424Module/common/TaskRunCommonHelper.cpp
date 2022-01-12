#include "TaskRunCommonHelper.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "StationResManagementSerialize.h"

/* 获取当前的站ID */
Optional<quint32> TaskRunCommonHelper::getStationCKUAC()
{
    using ResType = Optional<quint32>;
    StationResManagementMap stationResMap;
    if (!GlobalData::getStationResManagementInfo(stationResMap))
    {
        return ResType(ErrorCode::GetDataFailed, "获取站信息失败");
    }
    if (stationResMap.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "获取的站信息为空,请检查站资源管理是否有本站相关UAC地址");
    }
    auto info = stationResMap.first();
    return ResType(info.ttcUACAddr.toUInt(nullptr, 16));
}

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

SystemWorkMode TaskRunCommonHelper::trackingToSystemWorkMode(SatelliteTrackingMode mode)
{
    if (mode == _4426_STTC)
    {
        return STTC;
    }
    if (mode == _4426_SK2)
    {
        return Skuo2;
    }
    if (mode == _4426_SKT)
    {
        return SKT;
    }
    // 截止20211023 扩跳不能快速校相
    if (mode == _4426_KaCk ||       //
        mode == _4426_STTC_KaCk ||  //
        mode == _4426_SK2_KaCk)
    {
        return KaKuo2;
    }
    if (mode == _4426_KaDSDT ||            //
        mode == _4426_STTC_KaDSDT ||       //
        mode == _4426_SK2_KaDSDT ||        //
        mode == _4426_STTC_KaCk_KaDSDT ||  //
        mode == _4426_SK2_KaCk_KaDSDT)
    {
        return KaDS;
    }
    if (mode == _4426_KaGSDT ||            //
        mode == _4426_STTC_KaGSDT ||       //
        mode == _4426_SK2_KaGSDT ||        //
        mode == _4426_STTC_KaCK_KaGSDT ||  //
        mode == _4426_SK2_KaCK_KaGSDT)
    {
        return KaGS;
    }

    return NotDefine;
}

Optional<int> TaskRunCommonHelper::getACUSystemWorkMode(SatelliteTrackingMode mode)
{
    //这个参数很重要  1:S 2:Ka遥测 3:Ka数传 4:S+Ka遥测 5:S+Ka数传 6 S+Ka遥测+Ka数传
    using ResType = Optional<int>;
    if (mode == _4426_STTC ||  //
        mode == _4426_SK2 ||   //
        mode == _4426_SKT)
    {
        return ResType(1);
    }

    if (mode == _4426_KaCk)
    {
        return ResType(2);
    }

    if (mode == _4426_KaDSDT || mode == _4426_KaGSDT)
    {
        return ResType(3);
    }

    // 截止20211023 扩跳不能快速校相
    if (mode == _4426_STTC_KaCk ||  //
        mode == _4426_SK2_KaCk)
    {
        return ResType(4);
    }

    if (mode == _4426_STTC_KaDSDT ||  //
        mode == _4426_STTC_KaGSDT ||  //
        mode == _4426_SK2_KaDSDT ||   //
        mode == _4426_SK2_KaGSDT)
    {
        return ResType(5);
    }

    if (mode == _4426_STTC_KaCk_KaDSDT ||  //
        mode == _4426_STTC_KaCK_KaGSDT ||  //
        mode == _4426_SK2_KaCk_KaDSDT ||   //
        mode == _4426_SK2_KaCK_KaGSDT)
    {
        return ResType(6);
    }

    return ResType(ErrorCode::InvalidArgument, "获取ACU系统工作方式错误");
}

Optional<TaskTimeList> TaskRunCommonHelper::getDeviceWorkTaskInfo(const DeviceWorkTask& deviceWorkTask)
{
    using ResType = Optional<TaskTimeList>;
    if (deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "计划目标数据为空计划数据错误");
    }

    TaskTimeList taskTimeList;

    taskTimeList.m_deviceWorkTask = deviceWorkTask;
    taskTimeList.m_working_mode = deviceWorkTask.m_working_mode;

    // 获取主跟目标任务代号
    auto masterTaskCode = deviceWorkTask.m_lord_with_target;
    taskTimeList.masterTaskCode = masterTaskCode;
    taskTimeList.m_is_zero = deviceWorkTask.m_is_zero == TaskCalibration::CaliBrationOnce;
    taskTimeList.m_calibration = deviceWorkTask.m_calibration == TaskCalibration::CaliBrationOnce;

    for (auto& target : deviceWorkTask.m_totalTargetMap)
    {
        SatelliteManagementData satelliteTemp;
        if (!GlobalData::getSatelliteManagementData(target.m_task_code, satelliteTemp))
        {
            return ResType(ErrorCode::InvalidArgument, "自动化运行无法获取正确的任务代号");
        }

        // 当前目标是主跟目标，使用主跟目标的时间发上行、载波等
        if (target.m_task_code == masterTaskCode)
        {
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

        auto curTargetTaskCode = target.m_task_code;

        // 解析链路
        for (auto& link : target.m_linkMap)
        {
            TaskTimeEntity entity;
            entity.m_taskCode = curTargetTaskCode;
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
            taskTimeList.m_taskCode2SCCenterMap[curTargetTaskCode] = satelliteTemp.m_digitalCenterID;  // 更新数传中心标识
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
            if (curTargetTaskCode == masterTaskCode)
            {
                taskTimeList.masterTaskBz = satelliteTemp.m_satelliteIdentification;
            }
        }
    }

    if (taskTimeList.masterTaskCode.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前任务计划主跟目标为空计划错误");
    }

    //处理4424 S扩二+一体化 组合问题
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

    // 处理跟踪模式
    // 当前下发的任务模式和设置的跟踪模式对应不上时需要处理一下
    auto result = checkTrackingMode(taskTimeList);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }

    result = checkTrackingPointFreq(taskTimeList);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    return ResType(taskTimeList);
}

ACUPhaseCalibrationFreqBand TaskRunCommonHelper::trackingModeToAcuFreqBandStatus(SatelliteTrackingMode mode)
{
    if (mode == _4426_STTC ||  //
        mode == _4426_SK2 ||   //
        mode == _4426_SKT)
    {
        return ACUPhaseCalibrationFreqBand::S;
    }

    if (mode == _4426_KaCk)
    {
        return ACUPhaseCalibrationFreqBand::KaYC;
    }

    if (mode == _4426_KaDSDT || mode == _4426_KaGSDT)
    {
        return ACUPhaseCalibrationFreqBand::KaSC;
    }

    // 截止20211023 扩跳不能快速校相
    if (mode == _4426_STTC_KaCk ||  //
        mode == _4426_SK2_KaCk)
    {
        return ACUPhaseCalibrationFreqBand::KaYC;
    }

    if (mode == _4426_STTC_KaDSDT ||  //
        mode == _4426_STTC_KaGSDT ||  //
        mode == _4426_SK2_KaDSDT ||   //
        mode == _4426_SK2_KaGSDT)
    {
        return ACUPhaseCalibrationFreqBand::SKaSC;
    }

    if (mode == _4426_STTC_KaCk_KaDSDT ||  //
        mode == _4426_STTC_KaCK_KaGSDT ||  //
        mode == _4426_SK2_KaCk_KaDSDT ||   //
        mode == _4426_SK2_KaCK_KaGSDT)
    {
        return ACUPhaseCalibrationFreqBand::SKaYCKaSC;
    }
    return ACUPhaseCalibrationFreqBand::Unknown;
}

OptionalNotValue TaskRunCommonHelper::checkTrackingMode(TaskTimeList& taskTimeList)
{
    auto& trackMode = taskTimeList.trackingMode;
    auto& modeExist = taskTimeList.modeExist;

    if (trackMode == _4426_STTC && modeExist.sttcMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_SK2 && modeExist.skuo2Mode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_KaCk && modeExist.kakuo2Mode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_KaDSDT && modeExist.kadsMode)
    {
        return OptionalNotValue();
    }
    if (trackMode == _4426_KaGSDT && modeExist.kagsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_STTC_KaCk && modeExist.sttcMode && modeExist.kakuo2Mode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_SK2_KaCk && modeExist.skuo2Mode && modeExist.kakuo2Mode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_STTC_KaDSDT && modeExist.sttcMode && modeExist.kadsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_STTC_KaGSDT && modeExist.sttcMode && modeExist.kagsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_SK2_KaDSDT && modeExist.skuo2Mode && modeExist.kadsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_SK2_KaGSDT && modeExist.skuo2Mode && modeExist.kagsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_STTC_KaCk_KaDSDT && modeExist.sttcMode && modeExist.kakuo2Mode && modeExist.kadsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_STTC_KaCK_KaGSDT && modeExist.sttcMode && modeExist.kakuo2Mode && modeExist.kagsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_SK2_KaCk_KaDSDT && modeExist.skuo2Mode && modeExist.kakuo2Mode && modeExist.kadsMode)
    {
        return OptionalNotValue();
    }

    if (trackMode == _4426_SK2_KaCK_KaGSDT && modeExist.skuo2Mode && modeExist.kakuo2Mode && modeExist.kagsMode)
    {
        return OptionalNotValue();
    }

    // 截止20211023 扩跳不能快速校相
    //    if (mode == _4426_SKT && modeExist.sktMode)
    //    {
    //        return OptionalNotValue();
    //    }
    //    if (mode == _4426_SKT_KaCk && modeExist.sktMode && modeExist.kakuo2Mode)
    //    {
    //        return OptionalNotValue();
    //    }
    //    if (mode == _4426_SKT_KaDSDT && modeExist.sktMode && modeExist.kadsMode)
    //    {
    //        return OptionalNotValue();
    //    }

    //    if (mode == _4426_SKT_KaGSDT && modeExist.sktMode && modeExist.kagsMode)
    //    {
    //        return OptionalNotValue();
    //    }
    //    if (mode == _4426_SKT_KaCk_KaDSDT && modeExist.sktMode && modeExist.kakuo2Mode && modeExist.kadsMode)
    //    {
    //        return OptionalNotValue();
    //    }
    //    if (mode == _4426_SKT_KaCK_KaGSDT && modeExist.sktMode && modeExist.kakuo2Mode && modeExist.kagsMode)
    //    {
    //        return OptionalNotValue();
    //    }

    /* 没找到重新计算 */
    if (modeExist.kagsMode)
    {
        taskTimeList.trackingMode = _4426_KaGSDT;
        return OptionalNotValue();
    }
    if (modeExist.kadsMode)
    {
        taskTimeList.trackingMode = _4426_KaDSDT;
        return OptionalNotValue();
    }
    if (modeExist.kakuo2Mode)
    {
        taskTimeList.trackingMode = _4426_KaCk;
        return OptionalNotValue();
    }
    if (modeExist.skuo2Mode)
    {
        taskTimeList.trackingMode = _4426_SK2;
        return OptionalNotValue();
    }
    if (modeExist.sttcMode)
    {
        taskTimeList.trackingMode = _4426_STTC;
        return OptionalNotValue();
    }
    // 截止20211023 扩跳不能快速校相
    // if (modeExist.sktMode)
    // {
    //     taskTimeList.trackingMode = _4426_SKT;
    //     return OptionalNotValue();
    // }
    return OptionalNotValue(ErrorCode::InvalidArgument, "当前任务跟踪模式无对应的工作模式，且无法自动计算任务执行错误");
}

OptionalNotValue TaskRunCommonHelper::checkTrackingPointFreq(TaskTimeList& taskTimeList)
{
    auto workMode = TaskRunCommonHelper::trackingToSystemWorkMode(taskTimeList.trackingMode);
    if (workMode == SystemWorkMode::NotDefine)
    {
        return OptionalNotValue(ErrorCode::InvalidArgument, "计划解析错误跟踪对应的模式数据不存在");
    }

    auto entity = taskTimeList.m_dataMap.value(workMode);
    if (entity.isEmpty())
    {
        return OptionalNotValue(ErrorCode::InvalidArgument, "计划解析错误跟踪对应的模式数据不存在");
    }

    taskTimeList.trackingPointFreqNo = 1;  // 跟踪用哪个点频的数据 这里只要计划解析成功就是1
    return OptionalNotValue();
}
