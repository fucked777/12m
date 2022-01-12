#include "OnekeyXLHandler.h"

#include "DeviceProcessMessageSerialize.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "PKXLBPQBHLinkHandler.h"
#include "RedisHelper.h"
#include "SatelliteManagementDefine.h"
#include "UpLinkHandler.h"
#include "Utility.h"
#include <QApplication>

class OnekeyXLHandlerImpl
{
public:
    double distZeroMean = 0;
    bool isSuccess{ false };
    // 上下行频率
    QVariant upFrequency;
    QVariant downFrequency;
    QVariant curHPAW; /* 当前功放的功率 */
    // 主跟数据
    TargetInfo targetInfo;
    // 卫星数据
    SatelliteManagementData satData;
    // 旋向
    SystemOrientation systemOrientation = SystemOrientation::Unkonwn;
    // 校零测参数
    QVariantMap replaceParamMap;
    //
    CKJDControlInfo jdInfo;

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
    : BaseParamMacroHandler(parent)
    , m_impl(new OnekeyXLHandlerImpl)
{
}
OnekeyXLHandler::~OnekeyXLHandler() {}
SystemWorkMode OnekeyXLHandler::getSystemWorkMode() { return SystemWorkMode::NotDefine; }
/* 只做校零操作
 * 环路和参数宏自己下
 */
bool OnekeyXLHandler::handle()
{
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        emit signalErrorMsg(QString("校零只支持测控模式进行当前校零的模式为:%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode)));
        return false;
    }
    // 获取测控基带ID
    auto result = getCKJDControlInfo(mLinkLine.workMode);
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(result.msg());
    }
    if (!result)
    {
        return false;
    }
    auto info = result.value();
    m_impl->jdInfo = info;

    auto resultParam = m_impl->getParam(mLinkLine);
    if (!resultParam)
    {
        emit signalErrorMsg(resultParam.msg());
        return false;
    }
    emit signalInfoMsg(QString("当前工作上行工作频率%1MHz，下行工作频率%2MHz").arg(m_impl->upFrequency.toString(), m_impl->downFrequency.toString()));
    emit signalInfoMsg(QString("当前旋向%1").arg(SystemWorkModeHelper::systemOrientationToDesc(m_impl->systemOrientation)));

    // 上行链路
    emit signalInfoMsg("启用上行链路");
    UpLinkHandler upLinkHandler;
    upLinkHandler.setEnableHpa(EnableMode::Enable);
    upLinkHandler.setEnableJD(EnableMode::Enable);
    upLinkHandler.setEnableQD(EnableMode::Enable);
    upLinkHandler.setRFOutputMode(RFOutputMode::TX);
    upLinkHandler.enableControl(UpLinkDeviceControl::CK);
    upLinkHandler.setQDSendSource(CKQDSendSource::JD);
    upLinkHandler.setAdd(JDOutPut::All);
    upLinkHandler.setManualMessage(mManualMsg);
    upLinkHandler.setLinkLine(mLinkLine);
    upLinkHandler.setRunningFlag(mRunningFlag);

    connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &OnekeyXLHandler::signalSendToDevice);
    connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &OnekeyXLHandler::signalInfoMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &OnekeyXLHandler::signalWarningMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &OnekeyXLHandler::signalErrorMsg);

    upLinkHandler.handle();

    ExitCheck(false);

    // 获取上下行频率
    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        auto result = STTC_ZeroCalibration(info.afterMaster);
        restoreRWLink(info.afterMaster);
        STTC_ZeroCalibration_End(info.afterMaster);        
        emit signalSpecificTipsMsg(QString("一键校零结束"));
        return result;
    }
    // S扩二
    else if (mLinkLine.workMode == Skuo2)
    {
        if (mManualMsg.linkLineMap.contains(Skuo2) && mManualMsg.linkLineMap.contains(KaKuo2))
        {
            info.afterMaster = getChangeModeCKDeviceID(info.afterMaster, Skuo2);
        }
        auto result = SKuo2_ZeroCalibration(info.afterMaster);
        restoreRWLink(info.afterMaster);
        SKuo2_ZeroCalibration_End(info.afterMaster);        
        emit signalSpecificTipsMsg(QString("一键校零结束"));
        return result;
    }
    // Ka扩二
    else if (mLinkLine.workMode == KaKuo2)
    {
        if (mManualMsg.linkLineMap.contains(Skuo2) && mManualMsg.linkLineMap.contains(KaKuo2))
        {
            info.afterMaster = getChangeModeCKDeviceID(info.afterMaster, KaKuo2);
        }
        auto result = KaKuo2_ZeroCalibration(info.afterMaster);
        restoreRWLink(info.afterMaster);
        KaKuo2_ZeroCalibration_End(info.afterMaster);
        emit signalSpecificTipsMsg(QString("一键校零结束"));
        return result;
    }
    // 扩跳
    else if (mLinkLine.workMode == SKT)
    {
        auto result = SKT_ZeroCalibration(info.afterMaster);
        restoreRWLink(info.afterMaster);
        SKT_ZeroCalibration_End(info.afterMaster);
        emit signalSpecificTipsMsg(QString("一键校零结束"));
        return result;
    }

    emit signalErrorMsg(QString("当前校零模式错误:%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode)));
    emit signalSpecificTipsMsg(QString("一键校零结束"));
    return false;
}
bool OnekeyXLHandler::waitCalibrationResult(SystemWorkMode workMode, const PackCommand& pack)
{
    //循环获取校零结果
    bool success = false;
    int timeOut = GlobalData::getCmdTimeCount(50);
    auto cacheKey = DeviceProcessHelper::getCmdResultRedisKey(pack.deviceID, pack.cmdId);
    while (timeOut--)
    {
        if(isExit())
        {
            return false;
        }
        ExitCheck(false);
        QString json;

        if (RedisHelper::getInstance().getData(cacheKey, json) && !json.isEmpty())
        {
            success = true;
            CmdResult result;
            json >> result;
            m_impl->distZeroMean = result.m_paramdataMap.value("DistZeroMean").toDouble(); /* 距离零值均值 */
            // auto distZeroVar = result.m_paramdataMap.value("DistZeroVar").toDouble();   /* 距离零值方差 */
            auto msg = QString("%1模式校零完成:距离零值均值:%2m")
                           .arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode))
                           .arg(m_impl->distZeroMean, 0, 'f');
            emit signalSpecificTipsMsg(msg);
            break;
        }
        GlobalData::waitCmdTimeMS();
        QApplication::processEvents();
    }
    m_impl->isSuccess = success;
    return success;
}

bool OnekeyXLHandler::STTC_ZeroCalibration(const DeviceID& ckjdDeviceID)
{
    // auto taskCode = m_impl->targetInfo.taskCode;
    // auto pointFreqNo = m_impl->targetInfo.pointFreqNo;

    clearQueue();
    // 需要设置一些额外参数
    // 设置测控基带为校零模式
    mSingleCommandHelper.packSingleCommand("StepSTTC_XL", mPackCommand, ckjdDeviceID, mLinkLine.workMode);
    appendExecQueue(mPackCommand);

    // 1.校零变频器需要设置频率
    QMap<QString, QVariant> frequencyMap;
    frequencyMap["SBandDownFreq"] = m_impl->upFrequency;
    frequencyMap["CheckZeroOutSigFreq"] = m_impl->downFrequency;

    // S校零变频器开电、设置为校零模式、设置输入输出频率
    mSingleCommandHelper.packSingleCommand("StepSXL_START", mPackCommand, frequencyMap);
    appendExecQueue(mPackCommand);

    // 双捕 起扫 回零 距捕
    //mSingleCommandHelper.packSingleCommand("StepGFQHKG_JL_ON", mPackCommand);
    //waitExecSuccess(mPackCommand);

    // 上行载波输出
    mSingleCommandHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);

    DeviceID sHpaDeviceID;
    if (getSGGFDeviceID(sHpaDeviceID))
    {
        m_impl->curHPAW = GlobalData::getReportParamData(sHpaDeviceID, 1, "TransmPower");
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_SHPA_ON_POWER_RATE", mPackCommand, sHpaDeviceID);
        appendExecQueue(mPackCommand);
    }
    execQueue();

    // 双捕 起扫 回零 距捕
    // 捕获方式设置为自动
    mSingleCommandHelper.packSingleCommand("StepSTTC_BH_AUTO", mPackCommand, ckjdDeviceID);
    waitExecSuccess(mPackCommand);
    QThread::sleep(2);

    // 双捕开始
    mSingleCommandHelper.packSingleCommand("StepSTTC_SB_START", mPackCommand, ckjdDeviceID);
    waitExecSuccess(mPackCommand);

    // 根据旋向去确认单元ID
    // 载波锁定 左旋/右旋中频接收 2/3
    // 主音锁定 左旋/右旋测距 5/6
    int unitCarrierLockIndID = 3;
    int unitHostLockIndID = 6;
    if (m_impl->systemOrientation == SystemOrientation::LCircumflex)
    {
        unitCarrierLockIndID = 2;
        unitHostLockIndID = 5;
    }

    // 获取基带状态数据
    int timeOut = GlobalData::getCmdTimeCount(50);
    int isReadyZeroCalibration = false;
    QThread::sleep(10);
    emit signalInfoMsg("等待载波锁定，主音锁定");
    while (timeOut--)
    {
        if(isExit())
        {
            return false;
        }
        QApplication::processEvents();
        GlobalData::waitCmdTimeMS();
        ExitCheck(false);
        /*
         * 载波锁定
         * 载波输出
         * 测距加调
         * 侧音锁定
         */

        //获取载波是否锁定 单元号为2/3
        auto carrierLockInd = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, "CarrierLockInd").toInt();
        //获取测距主音是否锁定 单元号为5/6
        auto hostLockInd = GlobalData::getReportParamData(ckjdDeviceID, unitHostLockIndID, "HostLockInd").toInt();
        //判断距捕灯是否锁定
        auto trappState = GlobalData::getReportParamData(ckjdDeviceID, unitHostLockIndID, "TrappState").toInt();
        if (carrierLockInd == 1 && hostLockInd == 1 && trappState == 1)
        {
            emit signalInfoMsg("载波锁定，主音锁定，具备校零条件");
            isReadyZeroCalibration = true;
            break;
        }
    }
    if (!isReadyZeroCalibration)
    {
        emit signalErrorMsg("载波，主音状态检测失败，无法进行一键校零");
        return false;
    }

    //系统配置里，默认统计50个点
    auto& replaceParamMap = m_impl->replaceParamMap;
    replaceParamMap.clear();
    replaceParamMap["TaskIdent"] = m_impl->satData.m_satelliteIdentification;          //任务标识
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(mLinkLine.workMode, mManualMsg.configMacroData, devNum);
    replaceParamMap["EquipComb"] = QString::number(devNum, 16);                        //设备组合号
    replaceParamMap["UplinkFreq"] = m_impl->upFrequency;                               //上行频率
    replaceParamMap["DownFreq"] = m_impl->downFrequency;                               //下行频率
    replaceParamMap["CorrectValue"] = GlobalData::getAutorunPolicyData("STTCXZ", 50);  //修正值
    replaceParamMap["SignalRot"] = static_cast<int>(m_impl->systemOrientation);        //信号旋向 1：左旋 2：右旋

    //下发校零命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_STTC_Start", mPackCommand, ckjdDeviceID, replaceParamMap);
    if (!waitExecSuccess(mPackCommand))
    {
        return false;
    }

    //获取校零结果
    auto success = waitCalibrationResult(STTC, mPackCommand);
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
void OnekeyXLHandler::STTC_ZeroCalibration_End(const DeviceID& ckjdDeviceID)
{
    // 恢复校零之前的功率
    DeviceID sHpaDeviceID;
    if (getSGGFDeviceID(sHpaDeviceID) && m_impl->curHPAW.isValid())
    {
        QVariantMap replaceMap;
        replaceMap["TransmPower"] = m_impl->curHPAW;
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_SHPA_TransmPower", mPackCommand, sHpaDeviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }
    auto& replaceParamMap = m_impl->replaceParamMap;
    if (replaceParamMap.isEmpty())
    {
        return;
    }

    //下发校零停止命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_STTC_Stop", mPackCommand, ckjdDeviceID, replaceParamMap);
    waitExecSuccess(mPackCommand);
}

bool OnekeyXLHandler::SKuo2_ZeroCalibration(const DeviceID& ckjdDeviceID)
{
    // auto taskCode = m_impl->targetInfo.taskCode;
    auto targetNo = m_impl->targetInfo.targetNo;

    clearQueue();
    // 需要设置一些额外参数
    // 设置测控基带为校零模式
    mSingleCommandHelper.packSingleCommand("StepSkuo2_XL", mPackCommand, ckjdDeviceID, mLinkLine.workMode);
    appendExecQueue(mPackCommand);

    // 1.校零变频器需要设置频率
    QMap<QString, QVariant> frequencyMap;
    frequencyMap["SBandDownFreq"] = m_impl->upFrequency;
    frequencyMap["CheckZeroOutSigFreq"] = m_impl->downFrequency;

    // S校零变频器开电、设置为校零模式、设置输入输出频率
    mSingleCommandHelper.packSingleCommand("StepSXL_START", mPackCommand, frequencyMap);
    appendExecQueue(mPackCommand);

    // S校零RF开
    //mSingleCommandHelper.packSingleCommand("StepGFQHKG_JL_ON", mPackCommand);
    //waitExecSuccess(mPackCommand);

    // 设置基带上行载波输出
    mSingleCommandHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);

    // 加调
    mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_YK_UP", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_ZB_UP", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);

    DeviceID sHpaDeviceID;
    if (getSGGFDeviceID(sHpaDeviceID))
    {
        m_impl->curHPAW = GlobalData::getReportParamData(sHpaDeviceID, 1, "TransmPower");
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_SHPA_ON_POWER_RATE", mPackCommand, sHpaDeviceID);
        appendExecQueue(mPackCommand);
    }

    execQueue();

    // 根据旋向去确认单元ID
    auto unitCarrierLockIndID = 1;  // 载波锁定 公共单元
    auto unitTRangFrameIndID = 4;   // 测距单元 测距帧同步

    // 获取基带状态数据
    int timeOut = GlobalData::getCmdTimeCount(10);
    int isReadyZeroCalibration = false;
    QThread::sleep(5);
    emit signalInfoMsg("等待遥测载波锁定，测量载波锁定，测距帧同步锁定");
    while (timeOut-- && !isReadyZeroCalibration)
    {
        if(isExit())
        {
            return false;
        }
        QApplication::processEvents();
        GlobalData::waitCmdTimeMS();
        ExitCheck(false);
        /*
         * 扩频
         * 载波锁定
         * 载波输出
         * 伪码锁定
         */
        // 遥测载波 1:锁定 2失锁
        auto telemCarrLock = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, "K2_TelemCarrLock", targetNo).toInt();
        // 测量载波 1:锁定 2失锁
        auto meaCarrLock = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, "K2_MeaCarrLock", targetNo).toInt();
        // 测距帧同步 1:锁定 2失锁
        auto tRangFrameInd = GlobalData::getReportParamData(ckjdDeviceID, unitTRangFrameIndID, "K2_TRangFrameInd", targetNo).toInt();

        if (telemCarrLock == 1 && meaCarrLock == 1 && tRangFrameInd == 1)
        {
            emit signalInfoMsg("遥测载波锁定，测量载波锁定，测距帧同步锁定，具备校零条件");
            isReadyZeroCalibration = true;
            break;
        }
    }
    if (!isReadyZeroCalibration)
    {
        emit signalErrorMsg("测量载波，测距帧同步 检测失败，无法进行校零");
        return false;
    }

    //系统配置里，默认统计50个点
    auto& replaceParamMap = m_impl->replaceParamMap;
    replaceParamMap.clear();
    replaceParamMap["TaskIdent"] = m_impl->satData.m_satelliteIdentification;           //任务标识
    replaceParamMap["ChannelSel"] = targetNo;                                           // 1-4对应通道1234
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(mLinkLine.workMode, mManualMsg.configMacroData, devNum);
    replaceParamMap["EquipComb"] = QString::number(devNum, 16);                         //设备组合号
    replaceParamMap["UplinkFreq"] = m_impl->upFrequency;                                //上行频率
    replaceParamMap["DownFreq"] = m_impl->downFrequency;                                //下行频率
    replaceParamMap["CorrectValue"] = GlobalData::getAutorunPolicyData("SKUO2XZ", 50);  //修正值
    replaceParamMap["SignalRot"] = static_cast<int>(m_impl->systemOrientation);         //信号旋向 1：左旋 2：右旋 无效

    //下发校零命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_Start", mPackCommand, ckjdDeviceID, replaceParamMap);
    if (!waitExecSuccess(mPackCommand))
    {
        return false;
    }

    //获取校零结果
    auto success = waitCalibrationResult(Skuo2, mPackCommand);

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
void OnekeyXLHandler::SKuo2_ZeroCalibration_End(const DeviceID& ckjdDeviceID)
{
    // 恢复校零之前的功率
    DeviceID sHpaDeviceID;
    if (getSGGFDeviceID(sHpaDeviceID) && m_impl->curHPAW.isValid())
    {
        QVariantMap replaceMap;
        replaceMap["TransmPower"] = m_impl->curHPAW;
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_SHPA_TransmPower", mPackCommand, sHpaDeviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }

    auto& replaceParamMap = m_impl->replaceParamMap;
    if (replaceParamMap.isEmpty())
    {
        return;
    }

    //下发校零停止命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_Stop", mPackCommand, ckjdDeviceID, replaceParamMap);
    waitExecSuccess(mPackCommand);
}

bool OnekeyXLHandler::KaKuo2_ZeroCalibration(const DeviceID& ckjdDeviceID)
{
    // auto taskCode = m_impl->targetInfo.taskCode;
    auto targetNo = m_impl->targetInfo.targetNo;

    clearQueue();
    // 需要设置一些额外参数
    // 设置测控基带为校零模式
    mSingleCommandHelper.packSingleCommand("StepKakuo2_XL", mPackCommand, ckjdDeviceID, mLinkLine.workMode);
    appendExecQueue(mPackCommand);
    // 1.校零变频器需要设置频率
    QMap<QString, QVariant> frequencyMap;
    frequencyMap["SBandDownFreq"] = m_impl->upFrequency;
    frequencyMap["CheckZeroOutSigFreq"] = m_impl->downFrequency;

    // Ka校零变频器开机同时设置成校零模式
    mSingleCommandHelper.packSingleCommand("StepKaXL_START", mPackCommand, frequencyMap);
    appendExecQueue(mPackCommand);

    // 测控基带上行载波输出
    mSingleCommandHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);

    // 加调
    mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_YK_UP", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_ZB_UP", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);

    DeviceID kaHpaDeviceID;
    if (getKaGGFDeviceID(kaHpaDeviceID))
    {
        m_impl->curHPAW = GlobalData::getReportParamData(kaHpaDeviceID, 1, "TransmPower");
        // 设置Ka功放输出40db
        mSingleCommandHelper.packSingleCommand("StepKaHPA_ON_POWER_RATE", mPackCommand, kaHpaDeviceID);
        appendExecQueue(mPackCommand);
    }

    execQueue();

    // 根据旋向去确认单元ID
    auto unitCarrierLockIndID = 1;  // 载波锁定 公共单元
    auto unitTRangFrameIndID = 4;   // 测距单元 测距帧同步

    // 等待基带锁定
    int timeOut = GlobalData::getCmdTimeCount(10);
    int isReadyZeroCalibration = false;
    QThread::sleep(5);
    emit signalInfoMsg("等待遥测载波锁定，测量载波锁定，测距帧同步锁定");
    while (timeOut-- && !isReadyZeroCalibration)
    {
        if(isExit())
        {
            return false;
        }
        QApplication::processEvents();
        GlobalData::waitCmdTimeMS();
        ExitCheck(false);
        /*
         * 扩频
         * 载波锁定
         * 载波输出
         * 伪码锁定
         */

        // 遥测载波 1:锁定 2失锁
        auto telemCarrLock = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, "K2_TelemCarrLock", targetNo).toInt();
        // 测量载波 1:锁定 2失锁
        auto meaCarrLock = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, "K2_MeaCarrLock", targetNo).toInt();
        // 测距帧同步 1:锁定 2失锁
        auto tRangFrameInd = GlobalData::getReportParamData(ckjdDeviceID, unitTRangFrameIndID, "K2_TRangFrameInd", targetNo).toInt();

        if (telemCarrLock == 1 && meaCarrLock == 1 && tRangFrameInd == 1)
        {
            emit signalInfoMsg("遥测载波锁定，测量载波锁定，测距帧同步锁定，具备校零条件");
            isReadyZeroCalibration = true;
            break;
        }
    }
    if (!isReadyZeroCalibration)
    {
        emit signalErrorMsg("遥测载波，测量载波，测距帧同步 检测失败，无法进行校零");
        return false;
    }

    //系统配置里，默认统计50个点
    QMap<QString, QVariant> replaceParamMap;
    replaceParamMap["TaskIdent"] = m_impl->satData.m_satelliteIdentification;            //任务标识
    replaceParamMap["ChannelSel"] = targetNo;                                            // 1-4对应通道1234
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(mLinkLine.workMode, mManualMsg.configMacroData, devNum);
    replaceParamMap["EquipComb"] = QString::number(devNum, 16);                          //设备组合号
    replaceParamMap["UplinkFreq"] = m_impl->upFrequency;                                 //上行频率
    replaceParamMap["DownFreq"] = m_impl->downFrequency;                                 //下行频率
    replaceParamMap["CorrectValue"] = GlobalData::getAutorunPolicyData("KAKUO2XZ", 50);  //修正值
    replaceParamMap["SignalRot"] = static_cast<int>(m_impl->systemOrientation);          //信号旋向 1：左旋 2：右旋 无效

    //下发校零命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_Start", mPackCommand, ckjdDeviceID, replaceParamMap);
    if (!waitExecSuccess(mPackCommand))
    {
        return false;
    }

    //获取校零结果
    auto success = waitCalibrationResult(KaKuo2, mPackCommand);
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
void OnekeyXLHandler::KaKuo2_ZeroCalibration_End(const DeviceID& ckjdDeviceID)
{
    // 恢复校零之前的功率
    DeviceID kaHpaDeviceID;
    if (getKaGGFDeviceID(kaHpaDeviceID) && m_impl->curHPAW.isValid())
    {
        QVariantMap replaceMap;
        replaceMap["power"] = m_impl->curHPAW;
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_KaHPA_TransmPower", mPackCommand, kaHpaDeviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }
    auto& replaceParamMap = m_impl->replaceParamMap;
    if (replaceParamMap.isEmpty())
    {
        return;
    }

    //下发校零停止命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_Stop", mPackCommand, ckjdDeviceID, replaceParamMap);
    waitExecSuccess(mPackCommand);
}

bool OnekeyXLHandler::SKT_ZeroCalibration(const DeviceID& ckjdDeviceID)
{
    // auto taskCode = m_impl->targetInfo.taskCode;
    // auto targetNo = m_impl->targetInfo.targetNo;

    clearQueue();
    // 需要设置一些额外参数
    // 设置测控基带为校零模式
    mSingleCommandHelper.packSingleCommand("StepSKT_XL", mPackCommand, ckjdDeviceID, mLinkLine.workMode);
    appendExecQueue(mPackCommand);
    // 1.校零变频器需要设置频率
    QMap<QString, QVariant> frequencyMap;
    frequencyMap["SBandDownFreq"] = m_impl->upFrequency;
    frequencyMap["CheckZeroOutSigFreq"] = m_impl->downFrequency;

    // S校零变频器开电、设置为校零模式、设置输入输出频率
    mSingleCommandHelper.packSingleCommand("StepSXL_START", mPackCommand, frequencyMap);
    appendExecQueue(mPackCommand);

    // 测控基带基带上行载波输出
    mSingleCommandHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);

    DeviceID sHpaDeviceID;
    if (getSGGFDeviceID(sHpaDeviceID))
    {
        m_impl->curHPAW = GlobalData::getReportParamData(sHpaDeviceID, 1, "TransmPower");
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_SHPA_ON_POWER_RATE", mPackCommand, sHpaDeviceID);
        appendExecQueue(mPackCommand);
    }

    execQueue();

    /* 扩跳在调制器单元有个扩跳输出方式 可以选择使用短码或者长码 */
    auto result = GlobalData::getReportParamData(ckjdDeviceID, 0x03, "ExtendedJumpOutputMode");
    QString carrierLockIndKey;     // 载波锁定
    QString pseudoCodeLockIndKey;  // 伪码锁定
    QString frameLockIndKey;       // 帧同步锁定
    /* 1短 2长 3并行 */
    auto tempValue = result.toInt();
    if (tempValue == 1 || tempValue == 3)
    {
        carrierLockIndKey = "Status2";
        pseudoCodeLockIndKey = "Status1";
        frameLockIndKey = "Status4";
    }
    else if (tempValue == 2)
    {
        carrierLockIndKey = "Status8";
        pseudoCodeLockIndKey = "Status7";
        frameLockIndKey = "Status10";
    }
    else
    {
        emit signalErrorMsg("扩跳模式获取当前使用长码或者短码错误，无法进行校零");
        return false;
    }
    auto unitCarrierLockIndID = 2;

    // 获取基带状态数据
    int timeOut = GlobalData::getCmdTimeCount(50);
    int isReadyZeroCalibration = false;
    QThread::sleep(50);
    emit signalInfoMsg("等待载波锁定，帧同步锁定，伪码锁定");
    while (timeOut-- && !isReadyZeroCalibration)
    {
        if(isExit())
        {
            return false;
        }
        QApplication::processEvents();
        GlobalData::waitCmdTimeMS();
        ExitCheck(false);
        //获取载波锁定是否锁定 单元号为2 中频接收机单元
        auto carrierLockInd = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, carrierLockIndKey).toInt();
        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        auto pseudoCodeLock = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, pseudoCodeLockIndKey).toInt();
        //获取帧同步锁定是否锁定 单元号为2 中频接收机单元
        auto frameLock = GlobalData::getReportParamData(ckjdDeviceID, unitCarrierLockIndID, frameLockIndKey).toInt();

        if (carrierLockInd == 1 && pseudoCodeLock == 1 && frameLock == 1)
        {
            emit signalInfoMsg("载波锁定，帧同步锁定，伪码锁定，具备校零条件");
            isReadyZeroCalibration = true;
        }
    }
    if (!isReadyZeroCalibration)
    {
        emit signalErrorMsg("载波，帧同步，伪码状态检测失败，无法进行校零");
        return false;
    }

    //系统配置里，默认统计50个点
    auto& replaceParamMap = m_impl->replaceParamMap;
    replaceParamMap.clear();
    replaceParamMap["TaskIdent"] = m_impl->satData.m_satelliteIdentification;        //任务标识
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(mLinkLine.workMode, mManualMsg.configMacroData, devNum);
    replaceParamMap["EquipComb"] = QString::number(devNum, 16);                      //设备组合号
    replaceParamMap["CorrectValue"] = GlobalData::getAutorunPolicyData("KTXZ", 50);  //修正值
    //下发校零命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_SKT_Start", mPackCommand, ckjdDeviceID, replaceParamMap);
    if (!waitExecSuccess(mPackCommand))
    {
        return false;
    }

    //获取校零结果
    auto success = waitCalibrationResult(SKT, mPackCommand);
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
void OnekeyXLHandler::SKT_ZeroCalibration_End(const DeviceID& ckjdDeviceID)
{
    // 恢复校零之前的功率
    DeviceID sHpaDeviceID;
    if (getSGGFDeviceID(sHpaDeviceID) && m_impl->curHPAW.isValid())
    {
        QVariantMap replaceMap;
        replaceMap["TransmPower"] = m_impl->curHPAW;
        // 功放输出40db
        mSingleCommandHelper.packSingleCommand("Step_SHPA_TransmPower", mPackCommand, sHpaDeviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }

    auto& replaceParamMap = m_impl->replaceParamMap;
    if (replaceParamMap.isEmpty())
    {
        return;
    }

    //下发校零停止命令
    mSingleCommandHelper.packSingleCommand("StepMACBDC_SKT_Stop", mPackCommand, ckjdDeviceID, replaceParamMap);
    waitExecSuccess(mPackCommand);
}
bool OnekeyXLHandler::getResult(double& distZeroMean)
{
    distZeroMean = m_impl->distZeroMean;
    return m_impl->isSuccess;
}
void OnekeyXLHandler::jdParamRestore(const DeviceID& ckjdDeviceID)
{
    /* 这里策略直接下个参数宏吧去还原太麻烦了 */
    auto workMode = mLinkLine.workMode;
    UnitTargetParamMap unitTargetParamMap;
    auto modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);

    // 这里得用对应模式的LinkLine
    auto linkLine = mManualMsg.linkLineMap.value(workMode);
    auto targetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
    ParamMacroModeData paramMacroModeItem;
    if (!GlobalData::getParamMacroModeData(targetInfo.taskCode, targetInfo.workMode, paramMacroModeItem))
    {
        emit signalErrorMsg(QString("获取任务代号为%1的参数宏的%2模式参数失败").arg(targetInfo.taskCode, workModeDesc));
        return;
    }
    // 获取点频参数转换的下发结构
    if (!getUnitTargetParamMap(workMode, linkLine, ckjdDeviceID, unitTargetParamMap))
    {
        return;
    }

    // 需要获取测控基带设备参数，添加到下发的参数宏中
    QMap<int, QVariantMap> ckjdUnitParamMap;
    paramMacroModeItem.getDeviceUnitParamMap(ckjdDeviceID, ckjdUnitParamMap);
    appendDeviceParam(ckjdUnitParamMap, unitTargetParamMap);

    /* 设备组合号 */
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(workMode, mManualMsg.configMacroData, devNum);
    unitTargetParamMap[1][0]["EquipCombNumb"] = QString::number(devNum, 16);
    packGroupParamSetData(ckjdDeviceID, modeID, unitTargetParamMap, mPackCommand);
    waitExecSuccess(mPackCommand);
}
void OnekeyXLHandler::restoreRWLink(const DeviceID& ckjdDeviceID)
{
    emit signalSpecificTipsMsg("校零结束参数开始恢复");

    jdParamRestore(ckjdDeviceID);
    clearQueue();
    // 设置测控基带为任务方式
    mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, ckjdDeviceID, mLinkLine.workMode);
    appendExecQueue(mPackCommand);

    // 上天线禁止
    mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, ckjdDeviceID);
    appendExecQueue(mPackCommand);
    execQueue();

    UpLinkHandler upLinkHandler;
    upLinkHandler.setEnableHpa(EnableMode::Disable);
    upLinkHandler.setEnableJD(EnableMode::Ignore);
    upLinkHandler.setEnableQD(EnableMode::Ignore);
    upLinkHandler.setIgnoreOtherDevice(true);
    upLinkHandler.setManualMessage(mManualMsg);
    upLinkHandler.enableControl(UpLinkDeviceControl::CK);
    upLinkHandler.setLinkLine(mLinkLine);
    upLinkHandler.setRunningFlag(mRunningFlag);

    connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &OnekeyXLHandler::signalSendToDevice);
    connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &OnekeyXLHandler::signalInfoMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &OnekeyXLHandler::signalWarningMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &OnekeyXLHandler::signalErrorMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalSpecificTipsMsg, this, &OnekeyXLHandler::signalSpecificTipsMsg);

    upLinkHandler.handleSpecific();

    emit signalSpecificTipsMsg("校零结束参数恢复完成");
}
