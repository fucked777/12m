#include "SttcZeroCalibrationHandler.h"

#include "RedisHelper.h"
#include "SingleCommandHelper.h"

SttcZeroCalibrationHandler::SttcZeroCalibrationHandler(SystemWorkMode pSystemWorkMode)
    : BaseZeroCalibrationHandler(pSystemWorkMode)
{
}

bool SttcZeroCalibrationHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    auto m_ckDeviceMap = mTaskTimeList.m_ckDeviceMap;
    auto m_ckQdDeviceMap = mTaskTimeList.m_ckQdDeviceMap;
    auto m_hpaDeviceMap = mTaskTimeList.m_hpaDeviceMap;

    auto m_upFrequencyMap = mTaskTimeList.m_upFrequencyMap;
    auto m_downFrequencyMap = mTaskTimeList.m_downFrequencyMap;
    auto m_upFrequencyListMap = mTaskTimeList.m_upFrequencyListMap;
    auto m_downFrequencyListMap = mTaskTimeList.m_downFrequencyListMap;

    // 测控基带设置为偏馈校零
    if (mTaskTimeList.m_dataMap.contains(mSystemWorkMode))
    {
        auto ckDeviceID = m_ckDeviceMap[mSystemWorkMode];
        // TODO 获取s功放的功率
        QVariant TransmPower = 30;
        {
            // globalSystemData->getUnpackMessageData(m_hpaDeviceMap[STTC], 0xFFFF, 1, "TransmPower", TransmPower);
        }

        //获取任务代号工作的频率
        auto upFrequency = m_upFrequencyMap[mSystemWorkMode];
        auto downFrequency = m_downFrequencyMap[mSystemWorkMode];

        QMap<QString, QVariant> frequencyMap;
        frequencyMap["SBandDownFreq"] = upFrequency;
        frequencyMap["CheckZeroOutSigFreq"] = downFrequency;

        QByteArray array;
        // 设置S功放加电功率 校零默认使用40dbm  功放加电
        singleCmdHelper.packSingleCommand("Step_SHPA_ON_Freq", packCommand, m_hpaDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);

        // S射频开关网络下发 校零参数
        singleCmdHelper.packSingleCommand("Step_XL", packCommand, frequencyMap);
        waitExecSuccess(packCommand);

        // S射频开关网络 加电
        singleCmdHelper.packSingleCommand("StepSSPKGWL_ON_POWER", packCommand);
        waitExecSuccess(packCommand);

        //基带上天线
        // 下发测控基带功率上天线
        singleCmdHelper.packSingleCommand("Step_GLSTX", packCommand, m_ckDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);

        // 判断当前卫星是否需要进行校零 如果需要校零就进行链路校零，如果不能就不做校零操作，基带加载校零操作
        // 设置基带为校零模式
        // TODO 工作方式设置成偏馈校零变频器
        singleCmdHelper.packSingleCommand("StepSTTC_XL", packCommand, m_ckDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);

        // 上行载波输出
        singleCmdHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput", packCommand, m_ckDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);

        // 捕获方式设置为自动
        singleCmdHelper.packSingleCommand("StepSTTC_BH_AUTO", packCommand, m_ckDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);
        QThread::sleep(2);
        // 双捕开始
        singleCmdHelper.packSingleCommand("StepSTTC_SB_START", packCommand, m_ckDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);

        SystemOrientation systemOrientation = (SystemOrientation)mTaskTimeList.m_currentXuanXiangMap[mSystemWorkMode];
        // 如果是左右旋同时 默认使用左旋信号
        if (systemOrientation == SystemOrientation::LRCircumflex)
        {
            systemOrientation = SystemOrientation::LCircumflex;
        }
        else if (systemOrientation > SystemOrientation::LRCircumflex)
        {
            systemOrientation = SystemOrientation::LCircumflex;
        }
        int unitCarrierLockIndID, unitHostLockIndID;

        if (systemOrientation == SystemOrientation::LCircumflex)
        {
            unitCarrierLockIndID = 2;
            unitHostLockIndID = 5;
        }
        else
        {
            unitCarrierLockIndID = 3;
            unitHostLockIndID = 6;
        }

        notifyInfoLog(QString("当前旋向%1").arg(systemOrientation == SystemOrientation::RCircumflex ? "右旋" : "左旋"));

        notifyInfoLog(QString("当前校零倒计时%1").arg(mTaskTimeList.commonTaskConfig.zeroTime));

        // 获取基带状态数据
        int timeOut = mTaskTimeList.commonTaskConfig.zeroTime;

        //是否具备校零条件
        int isReadyZeroCalibration = false;
        //是否校零成功
        bool isZeroSuccess = false;
        while (timeOut-- && !isReadyZeroCalibration)
        {
            //获取左旋载波是否锁定 单元号为2
            auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
            //获取左旋测距主音是否锁定 单元号为5
            auto HostLockInd = GlobalData::getReportParamData(ckDeviceID, unitHostLockIndID, "HostLockInd");
            //判断距捕灯是否锁定
            auto TrappState = GlobalData::getReportParamData(ckDeviceID, unitHostLockIndID, "TrappState");
            if (!CarrierLockInd.isValid() && !HostLockInd.isValid())
            {
                if (CarrierLockInd.toInt() == 1 && HostLockInd.toUInt() == 1)
                {
                    notifyInfoLog("载波锁定，主音锁定，具备校零条件");
                    isReadyZeroCalibration = true;
                }
            }
            QThread::msleep(1000);
        }

        if (!isReadyZeroCalibration)
        {
            notifyInfoLog(
                QString("（基带状态检测）测控基带%1(主机) 距捕未锁定，次音未锁定").arg(m_ckDeviceMap[mSystemWorkMode].extenID == 1 ? "A" : "B"));
        }
        else
        {
            notifyInfoLog(QString("（基带状态检测）测控基带%1(主机) 距捕锁定，次音锁定，具备校零条件")
                              .arg(m_ckDeviceMap[mSystemWorkMode].extenID == 1 ? "A" : "B"));

            //系统配置里，默认统计50个点
            QMap<QString, QVariant> replaceParamMap;
            replaceParamMap["TaskIdent"] = mTaskTimeList.masterTaskBz;           // 任务标识
            replaceParamMap["EquipComb"] = 0;                                    // 设备组合号
            replaceParamMap["UplinkFreq"] = upFrequency;                         // 上行频率
            replaceParamMap["DownFreq"] = downFrequency;                         // 下行频率
            replaceParamMap["CorrectValue"] = 0;                                 // 修正值
            replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  // 信号旋向 1：左旋 2：右旋
            //下发校零命令
            singleCmdHelper.packSingleCommand("StepMACBDC_STTC_Start", packCommand, ckDeviceID, replaceParamMap);
            waitExecSuccess(packCommand);

            //循环获取校零结果
            timeOut = 30;
            auto cacheKey = DeviceProcessHelper::getCmdResultRedisKey(packCommand.deviceID, packCommand.cmdId);
            while (timeOut--)
            {
                QString json;

                if (RedisHelper::getInstance().getData(cacheKey, json))
                {
                    CmdResult result;
                    json >> result;
                    auto distZeroMean = result.m_paramdataMap.value("DistZeroMean").toDouble(); /* 距离零值均值 */
                    auto distZeroVar = result.m_paramdataMap.value("DistZeroVar").toDouble();   /* 距离零值方差 */

                    auto msg =
                        QString("校零完成:\n距离零值均值:bak.distZeroMean\n距离零值方差:%2").arg(distZeroMean, 0, 'f').arg(distZeroVar, 0, 'f');
                    notifyInfoLog(msg);

                    //下发校零停止命令
                    singleCmdHelper.packSingleCommand("StepMACBDC_STTC_Stop", packCommand, ckDeviceID, replaceParamMap);
                    waitExecSuccess(packCommand);
                    isZeroSuccess = true;
                    break;
                }
                QThread::msleep(1000);
            }
        }

        //添加功放去负载
        bool m_taskType_yc = 0;  // 任务类型 遥测
        bool m_taskType_yk = 0;  // 任务类型 遥控
        bool m_taskType_cg = 0;  // 任务类型 测轨
        bool m_taskType_sc = 0;  // 任务类型 数传
        if (mTaskTimeList.m_ckDeviceMap.contains(mSystemWorkMode))
        {
            for (auto item : mTaskTimeList.m_dataMap[mSystemWorkMode])
            {
                if (item.isMaster)
                {
                    m_taskType_yc = item.m_taskType_yc;
                    m_taskType_yk = item.m_taskType_yk;
                    m_taskType_cg = item.m_taskType_cg;
                    m_taskType_sc = item.m_taskType_sc;
                }
            }
        }
        //默认功放的功率
        QMap<QString, QVariant> TransmPowerMap;
        TransmPowerMap["TransmPower"] = TransmPower;
        //如果是遥测任务 ，功放去电，遥控，测轨，上行数传 都是需要开功放的
        if (m_taskType_yc == 1)
        {
            // S功放去电
            singleCmdHelper.packSingleCommand(QString("Step_SHPA_Power_Forbid"), packCommand, m_hpaDeviceMap[mSystemWorkMode]);
            waitExecSuccess(packCommand);

            singleCmdHelper.packSingleCommand(QString("Step_SHPA_TransmPower"), packCommand, m_hpaDeviceMap[mSystemWorkMode], TransmPowerMap);
            waitExecSuccess(packCommand);
        }
        else
        {
            // S功放加电，功率恢复
            singleCmdHelper.packSingleCommand(QString("Step_SHPA_Power_Allow"), packCommand, m_hpaDeviceMap[mSystemWorkMode]);
            waitExecSuccess(packCommand);

            singleCmdHelper.packSingleCommand(QString("Step_SHPA_TransmPower"), packCommand, m_hpaDeviceMap[mSystemWorkMode], TransmPowerMap);
            waitExecSuccess(packCommand);
        }

        //基带下天线
        singleCmdHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, m_ckDeviceMap[mSystemWorkMode]);
        waitExecSuccess(packCommand);

        //基带去载波
        singleCmdHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput_STOP", packCommand, m_ckDeviceMap[mSystemWorkMode], TransmPowerMap);
        waitExecSuccess(packCommand);
        if (isReadyZeroCalibration && isZeroSuccess)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}
