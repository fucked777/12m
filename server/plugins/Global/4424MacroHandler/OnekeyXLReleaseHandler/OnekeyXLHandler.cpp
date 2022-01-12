#include "OnekeyXLHandler.h"

#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "PKXLBPQBHLinkHandler.h"
#include "RedisHelper.h"
#include "SatelliteManagementDefine.h"
#include "SingleCommandHelper.h"

class OnekeyXLHandlerImpl
{
public:
    double distZeroMean = 0;
    double distZeroVar = 0;
    bool isSuccess{ false };
    // 上下行频率
    QVariant upFrequency;
    QVariant downFrequency;
    // 主跟数据
    TargetInfo targetInfo;
    // 卫星数据
    SatelliteManagementData satData;
    // 旋向
    SystemOrientation systemOrientation = SystemOrientation::Unkonwn;
    // 校零测参数
    QVariantMap replaceParamMap;
    // 校零还原的相关参数
    bool restoreRW{ false };
    SystemWorkMode upWorkMode{ SystemWorkMode::NotDefine };

public:
    OptionalNotValue getParam(const LinkLine& mLinkLine);
};

OptionalNotValue OnekeyXLHandlerImpl::getParam(const LinkLine& mLinkLine)
{
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        return OptionalNotValue(ErrorCode::InvalidArgument, "获取主用任务代号失败");
    }
    targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    auto taskCode = targetInfo.taskCode;
    auto pointFreqNo = targetInfo.pointFreqNo;

    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);
    ParamMacroModeData paramMacroModeData;
    if (!GlobalData::getParamMacroModeData(taskCode, mLinkLine.workMode, paramMacroModeData))
    {
        auto msg = QString("获取任务代号为%1 工作模式为%2 的参数宏错误")  //
                       .arg(taskCode, workModeDesc);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    // 获取下行的频率
    if (!MacroCommon::getDownFrequency(paramMacroModeData, pointFreqNo, downFrequency))
    {
        auto msg = QString("获取任务代号为%1 工作模式为%2 点频为%3的参数宏下行频率错误")  //
                       .arg(taskCode, workModeDesc)                                       //
                       .arg(pointFreqNo);

        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    // 获取上行的频率
    if (!MacroCommon::getUpFrequency(paramMacroModeData, pointFreqNo, upFrequency))
    {
        auto msg = QString("获取任务代号为%1 工作模式为%2 点频为%3的参数宏上行频率错误")  //
                       .arg(taskCode, workModeDesc)                                       //
                       .arg(pointFreqNo);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        auto msg = QString("获取任务代号为%1 的卫星数据失败")  //
                       .arg(taskCode);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    if (!satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation))
    {
        auto msg = QString("获取任务代号为%1 的接收旋向错误")  //
                       .arg(taskCode);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
    {
        systemOrientation = SystemOrientation::LCircumflex;
    }

    return OptionalNotValue();
}

OnekeyXLHandler::OnekeyXLHandler(QObject* parent)
    : BaseHandler(parent)
    , mImpl(new OnekeyXLHandlerImpl())
{
}
OnekeyXLHandler::~OnekeyXLHandler() {}

void OnekeyXLHandler::setRestoreRWLink(bool restore, SystemWorkMode upWorkMode)
{
    mImpl->restoreRW = restore;
    mImpl->upWorkMode = upWorkMode;
}

/* 只做校零操作
 * 环路和参数宏自己下
 */
bool OnekeyXLHandler::handle()
{
    //    // 一键校零配置校零环路
    //    LinkLine taskLinkLine = mLinkLine;
    //    taskLinkLine.linkType = LinkType::PKXLBPQBH;
    //    PKXLBPQBHLinkHandler handler;
    //    handler.setManualMessage(this->mManualMsg);
    //    connect(&handler, &BaseLinkHandler::signalSendToDevice, this, &OnekeyXLHandler::signalSendToDevice);
    //    connect(&handler, &BaseLinkHandler::signalInfoMsg, this, &OnekeyXLHandler::signalInfoMsg);
    //    connect(&handler, &BaseLinkHandler::signalWarningMsg, this, &OnekeyXLHandler::signalWarningMsg);
    //    connect(&handler, &BaseLinkHandler::signalErrorMsg, this, &OnekeyXLHandler::signalErrorMsg);
    //    handler.setManualMessage(mManualMsg);
    //    handler.setLinkLine(taskLinkLine);
    //    if (!handler.handle())
    //    {
    //        return false;
    //    }

    // 获取主用测控基带ID
    DeviceID ckDeviceID;
    if (!getCKJDDeviceID(ckDeviceID))
    {
        return false;
    }

    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        STTC_ZeroCalibration(ckDeviceID);
    }
    // S扩二
    else if (mLinkLine.workMode == Skuo2)
    {
        SKuo2_ZeroCalibration(ckDeviceID);
    }
    // 一体化上面级
    else if (mLinkLine.workMode == SYTHSMJ)
    {
        SYTHSMJ_ZeroCalibration(ckDeviceID);
    }
    // 一体化卫星
    else if (mLinkLine.workMode == SYTHWX)
    {
        SYTHWX_ZeroCalibration(ckDeviceID);
    }
    // 一体化高码率
    else if (mLinkLine.workMode == SYTHGML)
    {
        SYTHGML_ZeroCalibration(ckDeviceID);
    }
    // 一体化上面级+扩二共载波
    else if (mLinkLine.workMode == SYTHSMJK2GZB)
    {
        SYTHSMJK2GZB_ZeroCalibration(ckDeviceID);
    }
    // 一体化上面级+扩二不共载波
    else if (mLinkLine.workMode == SYTHSMJK2BGZB)
    {
        SYTHSMJK2BGZB_ZeroCalibration(ckDeviceID);
    }
    // 一体化卫星+扩二
    else if (mLinkLine.workMode == SYTHWXSK2)
    {
        SYTHWXSK2_ZeroCalibration(ckDeviceID);
    }
    // 一体化高码率+扩二
    else if (mLinkLine.workMode == SYTHGMLSK2)
    {
        SYTHGMLSK2_ZeroCalibration(ckDeviceID);
    }
    // 扩跳
    else if (mLinkLine.workMode == SKT)
    {
        SKT_ZeroCalibration(ckDeviceID);
    }

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 如果是测控模式
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        // 获取测控前端和测控基带的DeviceID ，用于配置光口网络的信号
        // 获取测控基带ID
        DeviceID ckDeviceID;
        if (!getCKJDDeviceID(ckDeviceID))
        {
            return false;
        }
        // 切换基带的工作模式
        if (!switchJDWorkMode(ckDeviceID, mLinkLine.workMode))
        {
            emit signalErrorMsg(QString("%1 切换模式失败，任务工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(ckDeviceID)));
            return false;
        }

        // 设置测控基带为任务方式
        singleCmdHelper.packSingleCommand("StepCKJD_RWFS", packCommand, ckDeviceID, mLinkLine.workMode);
        waitExecSuccess(packCommand);

        // 获取测控前端
        DeviceID ckqdDeviceID;

        if (!getCKQDDeviceID(ckqdDeviceID))
        {
            return false;
        }

        // 切换前端的工作方式为任务方式
        singleCmdHelper.packSingleCommand("StepQD_WORK_RWFS", packCommand, ckqdDeviceID);
        waitExecSuccess(packCommand);

        // 测控基带功率上天线禁止
        singleCmdHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置切换开关激励关
        singleCmdHelper.packSingleCommand("StepGFQHKG_JL_OFF", packCommand);
        waitExecSuccess(packCommand);

        DeviceID sHpaDeviceID;
        if (getSGGFDeviceID(sHpaDeviceID))
        {
            // 设置S功放去电
            singleCmdHelper.packSingleCommand("Step_SHPA_Power_Forbid", packCommand, sHpaDeviceID);
            waitExecSuccess(packCommand);
        }
    }
    emit signalInfoMsg("一键校零完成");
    return true;
}

bool OnekeyXLHandler::waitCalibrationResult(const PackCommand& pack)
{
    //循环获取校零结果
    bool success = false;
    int timeOut = 50;
    auto cacheKey = DeviceProcessHelper::getCmdResultRedisKey(pack.deviceID, pack.cmdId);
    while (timeOut--)
    {
        QString json;

        if (RedisHelper::getInstance().getData(cacheKey, json))
        {
            success = true;
            CmdResult result;
            json >> result;
            m_distZeroMean = result.m_paramdataMap.value("DistZeroMean").toDouble(); /* 距离零值均值 */
            // auto distZeroVar = result.m_paramdataMap.value("DistZeroVar").toDouble();   /* 距离零值方差 */
            auto msg = QString("校零完成:距离零值均值:%1m").arg(m_distZeroMean, 0, 'f');
            emit signalInfoMsg(msg);
            break;
        }
        QThread::msleep(1000);
    }
    m_isSuccess = success;
    return success;
}

bool OnekeyXLHandler::STTC_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
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
    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //        //获取左旋载波是否锁定 单元号为2
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取左旋测距主音是否锁定 单元号为5
        //        auto HostLockInd = GlobalData::getReportParamData(ckDeviceID, unitHostLockIndID, "HostLockInd");
        //        //判断距捕灯是否锁定
        //        auto TrappState = GlobalData::getReportParamData(ckDeviceID, unitHostLockIndID, "TrappState");
        //        if (!CarrierLockInd.isValid() && !HostLockInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && HostLockInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，主音锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //            }
        //        }
        //        QThread::msleep(1000);
        // 跳过状态检查
        break;
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["EquipComb"] = 0;                                    //设备组合号
    replaceParamMap["UplinkFreq"] = upFrequency;                         //上行频率
    replaceParamMap["DownFreq"] = downFrequency;                         //下行频率
    replaceParamMap["CorrectValue"] = 0;                                 //修正值
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //信号旋向 1：左旋 2：右旋

    //下发校零命令
    singleCmdHelper.packSingleCommand("StepMACBDC_STTC_Start", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    singleCmdHelper.packSingleCommand("StepMACBDC_STTC_Stop", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SKuo2_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }
    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
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
    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //        auto unitCarrierLockIndID = 1;
        //        QVariant TelemCarrLock;  // 遥测载波 1:锁定 2失锁
        //        QVariant MeaCarrLock;    // 测量载波 1:锁定 2失锁
        //        TelemCarrLock = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TelemCarrLock", targetInfo.targetNo);
        //        TelemCarrLock = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "MeaCarrLock", targetInfo.targetNo);

        //        auto unitTRangFrameIndID = 4;  // 测距单元 测距帧同步
        //        QVariant TRangFrameInd;
        //        TelemCarrLock = GlobalData::getReportParamData(ckDeviceID, unitTRangFrameIndID, "TRangFrameInd", targetInfo.targetNo);

        //        if (!TelemCarrLock.isValid() && !MeaCarrLock.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (TelemCarrLock.toInt() == 1 && MeaCarrLock.toUInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("遥测载波锁定，测量载波锁定，测距帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //                break;
        //            }
        //        }
        //        QThread::msleep(1000);
        break;
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalInfoMsg("遥测载波，测量载波，测距帧同步 检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["ChannelSel"] = targetInfo.targetNo;                 // 1-4对应通道1234
    replaceParamMap["EquipComb"] = 0;                                    //设备组合号
    replaceParamMap["UplinkFreq"] = upFrequency;                         //上行频率
    replaceParamMap["DownFreq"] = downFrequency;                         //下行频率
    replaceParamMap["CorrectValue"] = 0;                                 //修正值
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //信号旋向 1：左旋 2：右旋 无效

    //下发校零命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SK2_Start", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SK2_Stop", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHSMJ_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //                break;
        //            }
        //        }
        //        QThread::msleep(1000);
        break;
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJ_Start", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJ_Stop", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHWX_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //                break;
        //            }
        //        }
        //        QThread::msleep(1000);
        break;
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWX_Start", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWX_Stop", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHGML_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //                break;
        //            }
        //        }
        //        QThread::msleep(1000);
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGML_Start", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGML_Stop", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHSMJK2GZB_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //一体化数传
        if (mLinkLine.masterType == YTH_DT) {}

        //一体化扩频
        if (mLinkLine.masterType == YTH_KP) {}
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //            }
        //        }
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2GZB_DT_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2GZB_K2_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2GZB_DT_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }
    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2GZB_K2_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHSMJK2BGZB_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //一体化数传
        if (mLinkLine.masterType == YTH_DT) {}

        //一体化扩频
        if (mLinkLine.masterType == YTH_KP) {}
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //            }
        //        }
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2BGZB_DT_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2BGZB_K2_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2BGZB_DT_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }
    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2BGZB_K2_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHWXSK2_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //一体化数传
        if (mLinkLine.masterType == YTH_DT) {}

        //一体化扩频
        if (mLinkLine.masterType == YTH_KP) {}
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //            }
        //        }
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWXK2_DT_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWXK2_K2_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWXK2_DT_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }
    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWXK2_K2_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }
    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SYTHGMLSK2_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //一体化数传
        if (mLinkLine.masterType == YTH_DT) {}

        //一体化扩频
        if (mLinkLine.masterType == YTH_KP) {}
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //            }
        //        }
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;    //任务标识
    replaceParamMap["SignalRot"] = static_cast<int>(systemOrientation);  //旋向
    replaceParamMap["CorrectValue"] = 0;                                 //修正值

    //下发校零命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGMLK2_DT_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGMLK2_K2_Start", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    //一体化数传
    if (mLinkLine.masterType == YTH_DT)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGMLK2_DT_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }
    //一体化扩频
    if (mLinkLine.masterType == YTH_KP)
    {
        singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGMLK2_K2_Stop", packCommand, ckDeviceID, replaceParamMap);
        waitExecSuccess(packCommand);
    }
    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::SKT_ZeroCalibration(DeviceID ckDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    //获取主用任务代号
    if (!mLinkLine.targetMap.contains(mLinkLine.masterTargetNo))
    {
        emit signalErrorMsg("获取主用任务代号失败");
        // TODO 将来需要打开
        // return false;
    }
    TargetInfo targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);

    SatelliteManagementData satData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satData))
    {
        emit signalErrorMsg("卫星管理无法获取到对应卫星数据");
        // TODO 将来需要打开
        // return false;
    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, downFrequency);

    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(upFrequency.toString()).arg(downFrequency.toString()));

    SystemOrientation systemOrientation;
    satData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, systemOrientation);
    // 如果是左右旋同时 默认使用左旋信号
    if (systemOrientation == SystemOrientation::LRCircumflex)
        systemOrientation = SystemOrientation::LCircumflex;
    // int unitCarrierLockIndID = 2;

    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(systemOrientation)));

    // 获取基带状态数据
    int timeOut = 50;
    int isReadyZeroCalibration = false;
    QThread::msleep(5000);
    while (timeOut-- && !isReadyZeroCalibration)
    {
        //        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        //        auto CarrierLockInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "CarrierLockInd");
        //        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        //        auto TRangFrameInd = GlobalData::getReportParamData(ckDeviceID, unitCarrierLockIndID, "TRangFrameInd");

        //        if (!CarrierLockInd.isValid() && !TRangFrameInd.isValid())
        //        {
        //            if (CarrierLockInd.toInt() == 1 && TRangFrameInd.toUInt() == 1)
        //            {
        //                emit signalInfoMsg("载波锁定，帧同步锁定，具备校零条件");
        //                isReadyZeroCalibration = true;
        //            }
        //        }
        //        QThread::msleep(1000);
    }
    if (!isReadyZeroCalibration)
    {
        // emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        // TODO 将来需要打开
        // return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = satData.m_satelliteIdentification;  //任务标识
    replaceParamMap["EquipComb"] = 0;                                  //设备组合号
    replaceParamMap["CorrectValue"] = 0;                               //修正值
    //下发校零命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SKT_Start", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    //获取校零结果
    auto success = waitCalibrationResult(mPackCommand);
    //下发校零停止命令
    singleCmdHelper.packSingleCommand("StepMACBDC_SKT_Stop", packCommand, ckDeviceID, replaceParamMap);
    waitExecSuccess(packCommand);

    if (success)
    {
        emit signalInfoMsg("校零成功");
    }
    else
    {
        emit signalWarningMsg("校零超时，校零失败");
    }
    return success;
}

bool OnekeyXLHandler::getResult(double& distZeroMean)
{
    distZeroMean = this->m_distZeroMean;
    return this->m_isSuccess;
}
