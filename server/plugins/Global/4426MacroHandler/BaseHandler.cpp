#include "BaseHandler.h"

#include "AutoRunTaskMessageDefine.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "PlanRunMessageSerialize.h"
#include "RedisHelper.h"
#include "ResourceRestructuringMessageDefine.h"
#include "SatelliteManagementDefine.h"
#include "SqlPhaseCalibration.h"
#include "StationResManagementSerialize.h"
#include "SubscriberHelper.h"
#include <QThread>

SingleCommandHelper BaseHandler::mSingleCommandHelper = SingleCommandHelper(); /* 命令组包 */

int CKJDControlInfo::getNeedSwitchWorkMode() const { return afterDoubleMode ? BaseHandler::getCKJDDoubleModeID() : -1; }
BaseHandler::BaseHandler(QObject* parent)
    : QObject(parent)
    , mRunningFlag(nullptr)
{
}

void BaseHandler::setManualMessage(const ManualMessage& manualMsg) { mManualMsg = manualMsg; }

void BaseHandler::setLinkLine(const LinkLine& linkLine) { mLinkLine = linkLine; }

bool BaseHandler::getCKJDMasterNumber(int& number)
{
    if (!mManualMsg.configMacroData.getCKJDMasterNumber(mLinkLine.workMode, number))
    {
        emit signalErrorMsg("获取主用测控基带数量失败");
        return false;
    }
    return true;
}

bool BaseHandler::getGSJDMasterNumber(int& number)
{
    if (!mManualMsg.configMacroData.getGSJDMasterNumber(mLinkLine.workMode, number))
    {
        emit signalErrorMsg("获取主用高速基带数量失败");
        return false;
    }
    return true;
}

bool BaseHandler::getDSJDMasterNumber(int& number)
{
    if (!mManualMsg.configMacroData.getDSJDMasterNumber(mLinkLine.workMode, number))
    {
        emit signalErrorMsg("获取主用低速基带数量失败");
        return false;
    }
    return true;
}

bool BaseHandler::getCKJDDeviceID(DeviceID& deviceID, int no)
{
    mManualMsg.configMacroData.getCKJDDeviceID(mLinkLine.workMode, deviceID, no);

    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取测控基带失败");
        return false;
    }
    return true;
}

Optional<MasterSlaveDeviceIDInfo> BaseHandler::getGSJDDeviceID()
{
    using Restype = Optional<MasterSlaveDeviceIDInfo>;
    MasterSlaveDeviceIDInfo info;
    mManualMsg.configMacroData.getGSJDDeviceID(mLinkLine.workMode, info.master, 1);
    mManualMsg.configMacroData.getGSJDDeviceID(mLinkLine.workMode, info.slave, 2);

    if (info.master.isValid() && info.slave.isValid())
    {
        return Restype(info);
    }

    if (!info.master.isValid())
    {
        return Restype(ErrorCode::Success, "获取主用高速基带失败", info);
    }

    if (!info.slave.isValid())
    {
        return Restype(ErrorCode::Success, "获取备用高速基带失败", info);
    }

    return Restype(ErrorCode::InvalidData, "获取当前主备基带错误", info);
}

void BaseHandler::getGSJDDeviceIDS(QList<DeviceID>& deviceIDList)
{
    auto result = getGSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return;
    }
    auto info = result.value();

    if (info.master.isValid())
    {
        deviceIDList.append(info.master);
    }

    if (info.slave.isValid())
    {
        deviceIDList.append(info.slave);
    }
}

Optional<MasterSlaveDeviceIDInfo> BaseHandler::getDSJDDeviceID()
{
    using Restype = Optional<MasterSlaveDeviceIDInfo>;
    MasterSlaveDeviceIDInfo info;
    mManualMsg.configMacroData.getDSJDDeviceID(mLinkLine.workMode, info.master, 1);
    mManualMsg.configMacroData.getDSJDDeviceID(mLinkLine.workMode, info.slave, 2);

    if (info.master.isValid() && info.slave.isValid())
    {
        return Restype(info);
    }

    if (!info.master.isValid())
    {
        return Restype(ErrorCode::Success, "获取主用低速基带失败", info);
    }

    if (!info.slave.isValid())
    {
        return Restype(ErrorCode::Success, "获取备用低速基带失败", info);
    }

    return Restype(ErrorCode::InvalidData, "获取当前主备基带错误", info);
}

void BaseHandler::getDSJDDeviceIDS(QList<DeviceID>& deviceIDList)
{
    auto result = getDSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return;
    }
    auto info = result.value();

    if (info.master.isValid())
    {
        deviceIDList.append(info.master);
    }

    if (info.slave.isValid())
    {
        deviceIDList.append(info.slave);
    }
}
Optional<MasterSlaveDeviceIDInfo> BaseHandler::getCKQDDeviceID()
{
    using Restype = Optional<MasterSlaveDeviceIDInfo>;
    MasterSlaveDeviceIDInfo info;
    mManualMsg.configMacroData.getCKQDDeviceID(mLinkLine.workMode, info.master, 1);
    mManualMsg.configMacroData.getCKQDDeviceID(mLinkLine.workMode, info.slave, 2);
    // 测控数字化前端就只有4台
    // 1/2  是一组
    // 3/4  是一组
    if (info.master.extenID == 1 || info.master.extenID == 3)
    {
        info.slave = info.master;
        info.slave.extenID = info.master.extenID + 1;
        return Restype(info);
    }
    else if (info.master.extenID == 2 || info.master.extenID == 4)
    {
        info.slave = info.master;
        info.slave.extenID = info.master.extenID - 1;
        return Restype(info);
    }

    // 虽然到这里主用数字化前端已经获取到了
    // 但是还是报错因为到这里要不系统错误 要不配置错误  要不就增加了设备
    // 反正对于当前系统的执行是未知的 所以直接报错误就行
    return Restype(ErrorCode::UnknownError, "系统错误未能获取测控数字化前端的信息");
}

Optional<MasterSlaveDeviceIDInfo> BaseHandler::getGZQDDeviceID()
{
    using Restype = Optional<MasterSlaveDeviceIDInfo>;
    MasterSlaveDeviceIDInfo info;
    mManualMsg.configMacroData.getGZQDDeviceID(mLinkLine.workMode, info.master, 1);
    mManualMsg.configMacroData.getGZQDDeviceID(mLinkLine.workMode, info.slave, 2);
    // 跟踪数字化前端就只有4台
    // 1/2  是一组
    // 3/4  是一组
    if (info.master.extenID == 1 || info.master.extenID == 3)
    {
        info.slave = info.master;
        info.slave.extenID = info.master.extenID + 1;
        return Restype(info);
    }
    else if (info.master.extenID == 2 || info.master.extenID == 4)
    {
        info.slave = info.master;
        info.slave.extenID = info.master.extenID - 1;
        return Restype(info);
    }

    // 虽然到这里主用数字化前端已经获取到了
    // 但是还是报错因为到这里要不系统错误 要不配置错误  要不就增加了设备
    // 反正对于当前系统的执行是未知的 所以直接报错误就行
    return Restype(ErrorCode::UnknownError, "系统错误未能获取跟踪数字化前端的信息");
}

bool BaseHandler::getSGGFDeviceID(DeviceID& deviceID)
{
    mManualMsg.configMacroData.getSGGFDeviceID(mLinkLine.workMode, deviceID);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取S高功放失败");
        return false;
    }
    return true;
}

bool BaseHandler::getMasterGZJDDeviceID(DeviceID& deviceID)
{
    // 跟踪前端和跟踪基带一对一
    mManualMsg.configMacroData.getGZJDDeviceID(mLinkLine.workMode, deviceID);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取跟踪基带失败");
        return false;
    }
    return true;
}

bool BaseHandler::getKaGGFDeviceID(DeviceID& deviceID)
{
    mManualMsg.configMacroData.getKaGGFDeviceID(mLinkLine.workMode, deviceID);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取Ka测控高功放设备失败");
        return false;
    }
    return true;
}

bool BaseHandler::getKaDtGGFDeviceID(DeviceID& deviceID)
{
    mManualMsg.configMacroData.getKaDtGGFDeviceID(mLinkLine.workMode, deviceID);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取Ka数传高功放设备失败");
        return false;
    }
    return true;
}

bool BaseHandler::waitExecSuccess(const PackCommand& packCommand, qint32 ttl)
{
    ExitCheck(false);

    if (!packCommand.errorMsg.isEmpty())
    {
        emit signalErrorMsg(packCommand.errorMsg);
        return false;
    }

    if (packCommand.data.isEmpty())
    {
        emit signalErrorMsg(QString("系统组包失败，请检查配置，当前的设备ID：0x%1，模式Id：0x%2，命令Id：%3，命令类型：%4")
                                .arg(packCommand.deviceID.toHex(), QString::number(packCommand.modeId, 16).toUpper())
                                .arg(packCommand.cmdId)
                                .arg(DevProtocolEnumHelper::devMsgTypeToDescStr(packCommand.cmdType)));
        return false;
    }

    // 当前命令操作信息(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
    auto currentCmdOperatorInfo = (packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo);

    // 判断设备是否在线
//    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
//    {
//        auto msg = QString("%1设备离线，发送%2命令%3失败")
//                       .arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(),
//                            currentCmdOperatorInfo.isEmpty() ? "" : QString("(%1)").arg(currentCmdOperatorInfo));
//        emit signalErrorMsg(msg);
//        return false;
//    }
//    if (!GlobalData::getDeviceSelfControl(packCommand.deviceID))
//    {
//        auto msg = QString("%1设备分控，发送%2命令%3失败")
//                       .arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(),
//                            currentCmdOperatorInfo.isEmpty() ? "" : QString("(%1)").arg(currentCmdOperatorInfo));
//        emit signalErrorMsg(msg);
//        return false;
//    }

    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    // 先清除当前的数据
    RedisHelper::getInstance().setData(redisFindKey, "");
    // 发送数据
    emit signalSendToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    // 不需要等待响应的命令
    if (ttl <= 0)
    {
        return true;
    }

    // 从循环读取响应的ID
    ttl = GlobalData::getCmdTimeCount(ttl);
    while (ttl > 0)
    {
        ExitCheck(false);
        GlobalData::waitCmdTimeMS();
        --ttl;

        // 获取命令执行结果
        QString cmdResponceStr;
        if (!RedisHelper::getInstance().getData(redisFindKey, cmdResponceStr))
        {
            continue;
        }

        ControlCmdResponse cmdResponce;
        cmdResponceStr >> cmdResponce;

        // 如果是当前发送的命令响应
        if (cmdResponce.isValid() && cmdResponce.deviceID == packCommand.deviceID && cmdResponce.cmdId == packCommand.cmdId &&
            cmdResponce.cmdType == packCommand.cmdType)
        {
            auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
            auto message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);

            if (cmdResponce.responseResult == ControlCmdResponseType::Success)
            {
                emit signalInfoMsg(message);
            }
            else if (cmdResponce.responseResult == ControlCmdResponseType::SubControlRejected)
            {
                emit signalWarningMsg(message);
            }
            else
            {
                emit signalErrorMsg(message);
            }

            // 接收到响应就直接返回
            return true;
        }
    }
    // 超时未响应
    emit signalErrorMsg(QString("%1,超时未响应").arg(currentCmdOperatorInfo));

    return false;
}

void BaseHandler::clearQueue() { mPackCommandQueue.clear(); }

void BaseHandler::appendExecQueue(const PackCommand& packCommand) { mPackCommandQueue << packCommand; }

void BaseHandler::execQueue(int ttl)
{
    mSentCommandMap.clear();
    mSentCommandRedisMap.clear();
    int index = 0;
    for (const auto& packCommand : mPackCommandQueue)
    {
        ExitCheckVoid();

        if (!packCommand.errorMsg.isEmpty())
        {
            emit signalErrorMsg(packCommand.errorMsg);
            continue;
        }

        if (packCommand.data.isEmpty())
        {
            emit signalErrorMsg("系统组包失败，请检查配置");
            continue;
        }

        // 当前命令操作信息
        // auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;

        // 判断设备是否在线
//        if (!GlobalData::getDeviceOnline(packCommand.deviceID))
//        {
//            auto msg = QString("%1设备离线，发送%2命令%3失败").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
//            emit signalErrorMsg(msg);
//            continue;
//        }
        // 判断是否为本控
//        if (!GlobalData::getDeviceSelfControl(packCommand.deviceID))
//        {
//            auto msg = QString("%1不为本控，发送%2命令%3失败").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
//            emit signalErrorMsg(msg);
//            continue;
//        }

        auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
        // 先清除当前的数据
        RedisHelper::getInstance().setData(redisFindKey, "");

        // 发送数据
        emit signalSendToDevice(packCommand.data);

        // 记录发送的命令
        mSentCommandMap[index] = packCommand;
        mSentCommandRedisMap[index] = redisFindKey;
        index++;

        QThread::msleep(50);
        // mSentCommandList << packCommand;

        // 写入日志当前命令执行的操作(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
        //        currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
        //        emit signalInfoMsg(currentCmdOperatorInfo);
    }

    // 没有发送的命令直接返回
    if (mSentCommandRedisMap.isEmpty())
    {
        return;
    }

    // 等待超时
    ttl = GlobalData::getCmdTimeCount(ttl);
    while (ttl > 0)
    {
        ExitCheckVoid();
        GlobalData::waitCmdTimeMS();
        --ttl;
        auto keys = mSentCommandMap.keys();
        for (auto& keyIndex : keys)
        {
            auto redisFindKey = mSentCommandRedisMap[keyIndex];
            auto packCommand = mSentCommandMap[keyIndex];
            // 获取命令执行结果
            QString cmdResponceStr;
            if (!RedisHelper::getInstance().getData(redisFindKey, cmdResponceStr))
            {
                continue;
            }

            ControlCmdResponse cmdResponce;
            cmdResponceStr >> cmdResponce;

            // 如果是当前发送的命令响应
            if (cmdResponce.isValid() && cmdResponce.deviceID == packCommand.deviceID && cmdResponce.cmdId == packCommand.cmdId &&
                cmdResponce.cmdType == packCommand.cmdType)
            {
                auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
                auto currentCmdOperatorInfo = (packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo);

                auto message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);

                if (cmdResponce.responseResult == ControlCmdResponseType::Success)
                {
                    emit signalInfoMsg(message);
                }
                else if (cmdResponce.responseResult == ControlCmdResponseType::SubControlRejected)
                {
                    emit signalWarningMsg(message);
                }
                else
                {
                    emit signalErrorMsg(message);
                }
                mSentCommandMap.remove(keyIndex);
                mSentCommandRedisMap.remove(keyIndex);
            }
        }
        if (mSentCommandMap.isEmpty())
        {
            break;
        }
    }

    // 如果超过最大等待时间还没有响应就判断该命令执行超时
    for (const auto& sentCmd : mSentCommandMap.values())
    {
        ExitCheckVoid();

        // 写入日志当前命令执行的操作(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
        auto currentCmdOperatorInfo = sentCmd.operatorInfo.isEmpty() ? sentCmd.getSetParamDescValue() : sentCmd.operatorInfo;
        currentCmdOperatorInfo = QString("%1-%2: %3").arg(sentCmd.getCmdDeviceName(), sentCmd.getCmdName(), currentCmdOperatorInfo);
        // 超时未响应
        emit signalErrorMsg(QString("%1,超时未响应").arg(currentCmdOperatorInfo));
    }
}

static OptionalNotValue getCKJDControlInfoImpl(const DeviceID& deviceID, DeviceID& before, DeviceID& after, int& curModeID, bool& beforeDouble,
                                               SystemWorkMode needWorkMode, bool doubleMode)
{
    auto tempDeviceID = deviceID;
    auto online1 = GlobalData::getDeviceOnline(deviceID);
    bool online2 = false;
    auto deviceName1 = GlobalData::getExtensionName(deviceID);
    auto needWorkModeName = SystemWorkModeHelper::systemWorkModeToDesc(needWorkMode);
    if (deviceID.extenID == 1 || deviceID.extenID == 3)
    {
        tempDeviceID.extenID += 1;
        online2 = GlobalData::getDeviceOnline(tempDeviceID);
    }
    else if (deviceID.extenID == 2 || deviceID.extenID == 4)
    {
        tempDeviceID.extenID -= 1;
        online2 = GlobalData::getDeviceOnline(tempDeviceID);
    }
    // 都在线就是双模式
    beforeDouble = (online2 && online1);
    /* 两个模式都不在线 */
    if (!(online2 || online1))
    {
        return OptionalNotValue(ErrorCode::DevOffline, QString("%1设备离线,切换模式为%2失败").arg(deviceName1, needWorkModeName));
    }

    auto getDeviceIDFunc = [&](const DeviceID& deviceID1, const DeviceID& deviceID2) {
        curModeID = GlobalData::getOnlineDeviceModeId(deviceID1);
        if (!SystemWorkModeHelper::modeIdIsValid(curModeID))
        {
            return false;
        }
        before = deviceID1;
        /*
         * 双模式
         * 处理一下保证当是双模式的时候返回的切换后的ID一定是4001/4003
         */
        if (doubleMode)
        {
            after = deviceID1;
            if (after.extenID == 2 || after.extenID == 4)
            {
                after.extenID -= 1;
            }
            return true;
        }

        if (curModeID == SystemWorkModeHelper::systemWorkModeConverToModeId(needWorkMode))
        {
            after = deviceID1;
            return true;
        }

        // 模式为ka扩2
        if (needWorkMode == SystemWorkMode::KaKuo2)
        {
            after = deviceID1.extenID > deviceID2.extenID ? deviceID1 : deviceID2;
        }
        else
        {
            after = deviceID1.extenID > deviceID2.extenID ? deviceID2 : deviceID1;
        }

        return true;
    };
    if (online1 && getDeviceIDFunc(deviceID, tempDeviceID))
    {
        return OptionalNotValue();
    }
    if (online2 && getDeviceIDFunc(tempDeviceID, deviceID))
    {
        return OptionalNotValue();
    }

    return OptionalNotValue(ErrorCode::UnknownError, "系统错误未能获取测控基带的信息");
}
QString BaseHandler::getCKJDWorkModeDesc(SystemWorkMode needWorkMode, bool isDoubleMode)
{
    return isDoubleMode ? "S扩2+Ka扩2" : SystemWorkModeHelper::systemWorkModeToDesc(needWorkMode);
}
int BaseHandler::getCKJDDoubleModeID() { return 6; }
bool BaseHandler::isDoubleMode() const { return mManualMsg.isDoubleMode(); }
//根据配置宏设定的跟踪前端，下备用跟踪基带参数
DeviceID BaseHandler::getGZJDSlaveDeviceID(const DeviceID& masterDeviceID)
{
    auto slaveDeviceID = masterDeviceID;
    if (slaveDeviceID.extenID == 1)
    {
        slaveDeviceID.extenID = 2;
    }
    else if (slaveDeviceID.extenID == 2)
    {
        slaveDeviceID.extenID = 1;
    }
    else if (slaveDeviceID.extenID == 3)
    {
        slaveDeviceID.extenID = 4;
    }
    else  // if (slaveDeviceID.extenID == 4)
    {
        slaveDeviceID.extenID = 3;
    }
    return slaveDeviceID;
}
TBBEModeID BaseHandler::convertGZJDMode(const QSet<SystemWorkMode>& workModeSet)
{
    if (workModeSet.contains(STTC) && workModeSet.contains(KaGS))
    {
        return TBBEModeID::STTC_KaGS;
    }

    if ((workModeSet.contains(Skuo2) && workModeSet.contains(KaGS)) ||  //
        (workModeSet.contains(KaKuo2) && workModeSet.contains(KaGS)))
    {
        return TBBEModeID::KP_KaGS;
    }

    if (workModeSet.contains(STTC) && workModeSet.contains(KaDS))
    {
        return TBBEModeID::STTC_KaDS;
    }

    if ((workModeSet.contains(KaKuo2) && workModeSet.contains(KaDS)) || (workModeSet.contains(Skuo2) && workModeSet.contains(KaDS)))
    {
        return TBBEModeID::KP_KaDS;
    }

    if (workModeSet.contains(STTC))
    {
        return TBBEModeID::STTC;
    }

    if (workModeSet.contains(Skuo2) || workModeSet.contains(KaKuo2))
    {
        return TBBEModeID::KP;
    }

    if (workModeSet.contains(KaGS))
    {
        return TBBEModeID::KaGS;
    }

    if (workModeSet.contains(SKT))
    {
        return TBBEModeID::SKT;
    }

    if (workModeSet.contains(XDS) || workModeSet.contains(KaDS))
    {
        return TBBEModeID::KaDS;
    }

    return TBBEModeID::Unknown;
}

Optional<std::tuple<SatelliteTrackingMode, int>> BaseHandler::checkTrackingInfo(const ManualMessage& mManualMsg, SatelliteTrackingMode trackingModeIn)
{
    using ResType = Optional<std::tuple<SatelliteTrackingMode, int>>;
    std::tuple<SatelliteTrackingMode, int> info;
    auto& trackingMode = std::get<0>(info);
    trackingMode = trackingModeIn;
    auto& trackingPointFreq = std::get<1>(info);

    auto resultMode = checkTrackingMode(mManualMsg, trackingMode);
    if (!resultMode)
    {
        return ResType(resultMode.errorCode(), resultMode.msg());
    }
    trackingMode = resultMode.value();

    auto resultPointFreq = checkTrackingPointFreq(mManualMsg, trackingMode);
    if (!resultPointFreq)
    {
        return ResType(resultMode.errorCode(), resultMode.msg());
    }
    trackingPointFreq = resultPointFreq.value();
    return ResType(info);
}
Optional<SatelliteTrackingMode> BaseHandler::checkTrackingMode(const ManualMessage& mManualMsg, SatelliteTrackingMode trackingMode)
{
    using ResType = Optional<SatelliteTrackingMode>;
    bool existSTTC = mManualMsg.configMacroData.configMacroCmdModeMap.contains(STTC);
    bool existSKuo2 = mManualMsg.configMacroData.configMacroCmdModeMap.contains(Skuo2);
    bool existKaKuo2 = mManualMsg.configMacroData.configMacroCmdModeMap.contains(KaKuo2);
    bool existSKT = mManualMsg.configMacroData.configMacroCmdModeMap.contains(SKT);
    bool existKaDS = mManualMsg.configMacroData.configMacroCmdModeMap.contains(KaDS);
    bool existKaGS = mManualMsg.configMacroData.configMacroCmdModeMap.contains(KaGS);

    // 上面的是优先匹配
    if (trackingMode == _4426_STTC && existSTTC)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SK2 && existSKuo2)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_KaCk && existKaKuo2)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SKT && existSKT)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_KaDSDT && existKaDS)
    {
        return ResType(trackingMode);
    }
    if (trackingMode == _4426_KaGSDT && existKaGS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_STTC_KaCk && existSTTC && existKaKuo2)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SK2_KaCk && existSKuo2 && existKaKuo2)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_STTC_KaDSDT && existSTTC && existKaDS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_STTC_KaGSDT && existSTTC && existKaGS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SK2_KaDSDT && existSKuo2 && existKaDS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SK2_KaGSDT && existSKuo2 && existKaGS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_STTC_KaCk_KaDSDT && existSTTC && existKaKuo2 && existKaDS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_STTC_KaCK_KaGSDT && existSTTC && existKaKuo2 && existKaGS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SK2_KaCk_KaDSDT && existSKuo2 && existKaKuo2 && existKaDS)
    {
        return ResType(trackingMode);
    }

    if (trackingMode == _4426_SK2_KaCK_KaGSDT && existSKuo2 && existKaKuo2 && existKaGS)
    {
        return ResType(trackingMode);
    }

    // 截止20211023 扩跳不能快速校相
    //    if (trackingMode == _4426_SKT_KaCk && modeExist.sktMode && modeExist.kakuo2Mode)
    //    {
    //        return ResType(trackingMode);
    //    }
    //    if (trackingMode == _4426_SKT_KaDSDT && modeExist.sktMode && modeExist.kadsMode)
    //    {
    //        return ResType(trackingMode);
    //    }

    //    if (trackingMode == _4426_SKT_KaGSDT && modeExist.sktMode && modeExist.kagsMode)
    //    {
    //        return ResType(trackingMode);
    //    }
    //    if (trackingMode == _4426_SKT_KaCk_KaDSDT && modeExist.sktMode && modeExist.kakuo2Mode && modeExist.kadsMode)
    //    {
    //        return ResType(trackingMode);
    //    }
    //    if (trackingMode == _4426_SKT_KaCK_KaGSDT && modeExist.sktMode && modeExist.kakuo2Mode && modeExist.kagsMode)
    //    {
    //        return ResType(trackingMode);
    //    }

    /* 没找到重新计算
     * 20211120 以最大包含来计算
     */
    if (existSKuo2 && existKaKuo2 && existKaGS)
    {
        return ResType(_4426_SK2_KaCK_KaGSDT);
    }
    if (existSTTC && existKaKuo2 && existKaGS)
    {
        return ResType(_4426_STTC_KaCK_KaGSDT);
    }

    if (existSKuo2 && existKaKuo2 && existKaDS)
    {
        return ResType(_4426_SK2_KaCk_KaDSDT);
    }
    if (existSTTC && existKaKuo2 && existKaDS)
    {
        return ResType(_4426_STTC_KaCk_KaDSDT);
    }

    if (existSKuo2 && existKaKuo2)
    {
        return ResType(_4426_SK2_KaCk);
    }
    if (existSKuo2 && existKaGS)
    {
        return ResType(_4426_SK2_KaGSDT);
    }
    if (existSKuo2 && existKaDS)
    {
        return ResType(_4426_SK2_KaDSDT);
    }

    if (existSTTC && existKaKuo2)
    {
        return ResType(_4426_STTC_KaCk);
    }
    if (existSTTC && existKaGS)
    {
        return ResType(_4426_STTC_KaGSDT);
    }
    if (existSTTC && existKaDS)
    {
        return ResType(_4426_STTC_KaDSDT);
    }

    if (existKaGS)
    {
        return ResType(_4426_KaGSDT);
    }
    if (existKaDS)
    {
        return ResType(_4426_KaDSDT);
    }
    if (existKaKuo2)
    {
        return ResType(_4426_KaCk);
    }
    if (existSKuo2)
    {
        return ResType(_4426_SK2);
    }
    if (existSTTC)
    {
        return ResType(_4426_STTC);
    }
    // 截止20211023 扩跳不能快速校相
    if (existSKT)
    {
        return ResType(_4426_SKT);
    }
    return ResType(ErrorCode::InvalidArgument, "当前任务跟踪模式无对应的工作模式，且无法自动计算任务执行错误");
}

Optional<int> BaseHandler::checkTrackingPointFreq(const ManualMessage& mManualMsg, SatelliteTrackingMode trackingMode)
{
    using ResType = Optional<int>;
    auto workMode = trackingToSystemWorkMode(trackingMode);
    if (workMode == SystemWorkMode::NotDefine)
    {
        auto msg = QString("跟踪对应的模式不存在:%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return ResType(ErrorCode::InvalidArgument, msg);
    }

    auto modeItem = mManualMsg.linkLineMap.value(workMode);
    if (modeItem.targetMap.isEmpty())
    {
        auto msg = QString("跟踪对应的模式不存在:%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return ResType(ErrorCode::InvalidArgument, msg);
    }
    if (!modeItem.targetMap.contains(modeItem.masterTargetNo))
    {
        auto msg = QString("跟踪对应的模式不存在:%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return ResType(ErrorCode::InvalidArgument, msg);
    }

    return ResType(modeItem.targetMap.value(modeItem.masterTargetNo).pointFreqNo);
}
Optional<LinkType> BaseHandler::manualMessageTaskCheckOperation(const ManualMessage& msg)
{
    using ResType = Optional<LinkType>;
    if (msg.linkLineMap.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "配置错误当前链路数据为空");
    }
    /* 射频模拟源 和 联试应答机 会占用18口 所以不能存在多模式 */
    QSet<SystemWorkMode> workModeSet;
    QSet<LinkType> linkTypeSet;
    for (auto iter = msg.linkLineMap.begin(); iter != msg.linkLineMap.end(); ++iter)
    {
        workModeSet << iter.key();
        linkTypeSet << iter->linkType;
    }
    if (linkTypeSet.size() >= 2)
    {
        return ResType(ErrorCode::InvalidArgument, "当前不支持多种系统工作链路混合下发");
    }
    auto linkType = *linkTypeSet.begin();
    if (linkType == LinkType::Unknown)
    {
        return ResType(ErrorCode::InvalidArgument, "系统错误未知的系统链路类型");
    }
    if (workModeSet.size() >= 2 && linkType == LinkType::LSYDJYXBH)
    {
        return ResType(ErrorCode::InvalidArgument, "联试应答机闭环因为占用S接收矩阵X18口,故不能多种模式组合");
    }
    if (workModeSet.size() >= 2 && (linkType == LinkType::SPMNYWXBH || linkType == LinkType::SPMNYYXBH))
    {
        return ResType(ErrorCode::InvalidArgument, "射频模拟源闭环因为占用S接收矩阵X18口,故不能多种模式组合");
    }
    /* 联试应答机闭环只支持能发上行的模式 */
    auto tempWorkMode = *workModeSet.begin();
    if (linkType == LinkType::LSYDJYXBH && (tempWorkMode == XDS || tempWorkMode == KaGS))
    {
        return ResType(ErrorCode::InvalidArgument, "射频模拟源闭环部支持X低速或者Ka高速");
    }
    /* 无论配置宏还是参数宏 只要是任务方式就是任务开始 */
    if (msg.isTest)
    {
        GlobalData::cleanTaskRunningInfo();
    }
    else
    {
        GlobalData::setTaskRunningInfo(msg);
    }

    return ResType(linkType);
}

Optional<CKJDControlInfo> BaseHandler::getCKJDControlInfo(const ConfigMacroData& configMacroData, SystemWorkMode needWorkMode)
{
    /*
     * 这里已经处理过 双模式无论是传入哪个模式得到的数据都是一样的
     */
    using ResType = Optional<CKJDControlInfo>;
    CKJDControlInfo info;
    info.afterDoubleMode = (configMacroData.configMacroCmdModeMap.contains(Skuo2) && configMacroData.configMacroCmdModeMap.contains(KaKuo2));
    // 获取测控基带ID
    DeviceID deviceIDMaster;

    configMacroData.getCKJDDeviceID(needWorkMode, deviceIDMaster, 1);

    if (!deviceIDMaster.isValid())
    {
        return ResType(ErrorCode::InvalidData, "获取主用测控基带失败");
    }

    auto result1 = getCKJDControlInfoImpl(deviceIDMaster, info.beforeMaster, info.afterMaster, info.curModeIDMaster, info.beforeMasterDoubleMode,
                                          needWorkMode, info.afterDoubleMode);

    DeviceID deviceIDSlave;
    configMacroData.getCKJDDeviceID(needWorkMode, deviceIDSlave, 2);
    if (!deviceIDSlave.isValid())
    {
        deviceIDSlave = deviceIDMaster;
        if (deviceIDMaster.extenID == 1 || deviceIDMaster.extenID == 2)
        {
            deviceIDSlave.extenID += 2;
        }
        else
        {
            deviceIDSlave.extenID -= 2;
        }
    }

    auto result2 = getCKJDControlInfoImpl(deviceIDSlave, info.beforeSlave, info.afterSlave, info.curModeIDSlave, info.beforeSlaveDoubleMode,
                                          needWorkMode, info.afterDoubleMode);

    if (!result1 && result2)
    {
        return ResType(ErrorCode::InvalidData, result1.msg());
    }

    return ResType(ErrorCode::Success, result1 ? result2.msg() : result1.msg(), info);
}
Optional<CKJDControlInfo> BaseHandler::getCKJDControlInfo(SystemWorkMode needWorkMode)
{
    return getCKJDControlInfo(mManualMsg.configMacroData, needWorkMode);
}

CalibResultInfo BaseHandler::getCalibResultInfo(const QString& taskCode, int dotDrequency, SystemWorkMode workMode, quint64 equipComb,
                                                bool defaultValue)
{
    auto result = SqlPhaseCalibration::selectItem(taskCode, dotDrequency, workMode, equipComb);
    if (defaultValue || !result || result->isEmpty())
    {
        CalibResultInfo info;

        info.taskCode = taskCode;
        info.workMode = workMode;
        info.dotDrequency = dotDrequency;
        info.equipComb = equipComb;
        info.azpr = 0;
        info.elpr = 0;
        info.azge = 0.1;
        info.elge = 0.1;
        return info;
    }
    auto item = result->first();
    CalibResultInfo info;
    info.taskCode = taskCode;
    info.workMode = workMode;
    info.dotDrequency = dotDrequency;
    info.equipComb = equipComb;

    info.azpr = item.azpr;
    info.elpr = item.elpr;
    info.azge = item.azge;
    info.elge = item.elge;
    return info;
}

void BaseHandler::updateCalibResultInfoList(const CalibResultInfo& info)
{
    // 校相接口更新

    PhaseCalibrationItem item;
    item.projectCode = ExtendedConfig::curProjectID();
    item.taskCode = info.taskCode;
    item.workMode = info.workMode;
    item.dotDrequency = info.dotDrequency;
    item.equipComb = info.equipComb;
    item.downFreq = info.downFreq;

    item.azpr = info.azpr;
    item.elpr = info.elpr;
    item.azge = info.azge;
    item.elge = info.elge;
    SqlPhaseCalibration::insertItem(item);
}

BaseHandler::CKJDSwitchStatus BaseHandler::switchJDWorkMode(const CKJDControlInfo& ckjdControlInfo, SystemWorkMode needWorkMode)
{
    /* 主机 */
    auto masterRes = switchJDWorkModeNotWait(ckjdControlInfo, needWorkMode, true);
    /* 备机 */
    auto slaveRes = switchJDWorkModeNotWait(ckjdControlInfo, needWorkMode, false);
    if (CKJDItemSwitchStatus::Success == masterRes && CKJDItemSwitchStatus::Success == slaveRes)
    {
        emit signalInfoMsg(QString("测控基带当前模式正确，跳过模式切换"));
        return CKJDSwitchStatus::All;
    }
    if (CKJDItemSwitchStatus::Failed == masterRes && CKJDItemSwitchStatus::Failed == slaveRes)
    {
        return CKJDSwitchStatus::Failed;
    }

    auto deviceName = GlobalData::getExtensionName(ckjdControlInfo.beforeMaster);
    auto needWorkModeName = getCKJDWorkModeDesc(needWorkMode, ckjdControlInfo.afterDoubleMode);
    auto needModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(needWorkMode);
    // 切换之后的设备ID如果是双模式  被设置成4001/4003 这里也把模式切换为S的
    needModeId = ckjdControlInfo.afterDoubleMode ? SystemWorkModeHelper::systemWorkModeConverToModeId(Skuo2) : needModeId;

    emit signalInfoMsg(QString("正在切换%1模式为%2").arg(deviceName, needWorkModeName));

    // 等待退出
    /* 这里要首先清除after的状态 否则直接会误读状态
     * 设备模式存的是哈希表 redis没有过期时间 所以无论啥情况都给清掉
     */
#if 1
    if ((ckjdControlInfo.beforeMasterDoubleMode || ckjdControlInfo.beforeSlaveDoubleMode) &&
        (ckjdControlInfo.afterMaster.extenID == 2 || ckjdControlInfo.afterMaster.extenID == 4))
    {
        QThread::sleep(15);
    }
    else
    {
        QThread::sleep(10);
    }
#else
    QThread::sleep(5);
#endif

    GlobalData::setOnlineDeviceModeId(ckjdControlInfo.afterMaster, -1);
    GlobalData::setOnlineDeviceModeId(ckjdControlInfo.afterSlave, -1);

    auto tempDevice = getEqModeCKDeviceID(ckjdControlInfo.afterMaster);
    GlobalData::setOnlineDeviceModeId(tempDevice, -1);
    tempDevice = getEqModeCKDeviceID(ckjdControlInfo.afterSlave);
    GlobalData::setOnlineDeviceModeId(tempDevice, -1);

    //  等待60s判断模式是否切换成功
    int waitTime = GlobalData::getCmdTimeCount(60);
    bool successMaster = false;
    bool successSlave = false;
    for (auto i = 0; i < waitTime; ++i)
    {
        ExitCheck(CKJDSwitchStatus::Failed);
        GlobalData::waitCmdTimeMS();
        // 切换之后的设备ID如果是双模式  被设置成4001/4003
        auto curModeIdMaster = GlobalData::getOnlineDeviceModeId(ckjdControlInfo.afterMaster);
        auto curModeIdSlave = GlobalData::getOnlineDeviceModeId(ckjdControlInfo.afterSlave);
        if (curModeIdMaster == needModeId && !successMaster)
        {
            emit signalInfoMsg(QString("测控基带主机:%1模式切换成功").arg(GlobalData::getExtensionName(ckjdControlInfo.afterMaster)));
            successMaster = true;
        }
        if (curModeIdSlave == needModeId && !successSlave)
        {
            emit signalInfoMsg(QString("测控基带备机:%1模式切换成功").arg(GlobalData::getExtensionName(ckjdControlInfo.afterSlave)));
            successSlave = true;
        }
        if ((successMaster || masterRes == CKJDItemSwitchStatus::Failed) &&  //
            (successSlave || slaveRes == CKJDItemSwitchStatus::Failed))
        {
            // 等待5s,测控基带再切换模式之后界面未显示之前就已经上报数据了
            // 等待3s 容错等另一台机器启动完成
            QThread::sleep(8);
            break;
        }
    }
    return successMaster ? (successSlave ? CKJDSwitchStatus::All : CKJDSwitchStatus::Master)
                         : (successSlave ? CKJDSwitchStatus::Slave : CKJDSwitchStatus::Failed);
}
BaseHandler::CKJDItemSwitchStatus BaseHandler::switchJDWorkModeNotWait(const CKJDControlInfo& deviceInfo, SystemWorkMode needWorkMode, bool master)
{
    auto beforeDeviceID = master ? deviceInfo.beforeMaster : deviceInfo.beforeSlave;
    // auto afterDeviceID = master ? deviceInfo.afterMaster : deviceInfo.aftereSlave;
    auto curModeID = master ? deviceInfo.curModeIDMaster : deviceInfo.curModeIDSlave;
    auto beforeIsDouble = master ? deviceInfo.beforeMasterDoubleMode : deviceInfo.beforeSlaveDoubleMode;
    int needModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(needWorkMode);
    // 切换前后都是双模式
    if (deviceInfo.afterDoubleMode && beforeIsDouble)
    {
        return CKJDItemSwitchStatus::Success;
    }
    // 设备当前模式和需要的模式一致，不进行切换
    // 如果切换之前为双模式那么无论如何都要进行一次切换检查
    if (curModeID == needModeId && !(deviceInfo.afterDoubleMode || beforeIsDouble))
    {
        return CKJDItemSwitchStatus::Success;
    }

    auto deviceName = GlobalData::getExtensionName(beforeDeviceID);
    auto needWorkModeName = getCKJDWorkModeDesc(needWorkMode, deviceInfo.afterDoubleMode);

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(beforeDeviceID))
    {
        auto msg = QString("%1设备离线，切换模式为%2失败").arg(deviceName, needWorkModeName);
        emit signalErrorMsg(msg);
        return CKJDItemSwitchStatus::Failed;
    }
    // 判断是否为本控
    if (!GlobalData::getDeviceSelfControl(beforeDeviceID))
    {
        auto msg = QString("%1不为本控，切换模式为%2失败").arg(deviceName, needWorkModeName);
        emit signalErrorMsg(msg);
        return CKJDItemSwitchStatus::Failed;
    }

    // 发送工作模式切换命令
    QString singleCmdId;
    if (deviceInfo.afterDoubleMode)
    {
        //主要用作切换混合模式
        singleCmdId = QString("StepJD_WORK_MODE_SWITCH_%1").arg(getCKJDDoubleModeID());
    }
    else
    {
        singleCmdId = QString("StepJD_WORK_MODE_SWITCH_%1").arg(needWorkMode);
    }

    if (!mSingleCommandHelper.packSingleCommand(singleCmdId, mPackCommand, beforeDeviceID))
    {
        auto msg = QString("%1设备组包失败: %2").arg(deviceName, mPackCommand.errorMsg);
        emit signalErrorMsg(msg);
        return CKJDItemSwitchStatus::Failed;
    }
    // 发送命令，不等待  判断是否发送成功
    return waitExecSuccess(mPackCommand, 0) ? CKJDItemSwitchStatus::CmdSuccess : CKJDItemSwitchStatus::Failed;
}

bool BaseHandler::switchGZJDWorkModeNotWait(const DeviceID& deviceID, int needSwitchWorkMode)
{
    auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
    // 设备当前模式和需要的模式一致，不进行切换
    if (curModeId == needSwitchWorkMode)
    {
        return true;
    }

    auto deviceName = GlobalData::getExtensionName(deviceID);

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(deviceID))
    {
        auto msg = QString("%1设备离线，切换模式失败").arg(deviceName);
        emit signalErrorMsg(msg);
        return false;
    }
    // 判断是否为本控
    if (!GlobalData::getDeviceSelfControl(deviceID))
    {
        auto msg = QString("%1不为本控，切换模式失败").arg(deviceName);
        emit signalErrorMsg(msg);
        return false;
    }

    // 发送工作模式切换命令  StepGZJD_WorkMode_STTC这个命令只要设备ID和切换模式正确，所有基带工作模式命令号为101的都能用
    QMap<QString, QVariant> params;
    params["WorkMode"] = needSwitchWorkMode;

    emit signalInfoMsg(QString("%1正在切换模式").arg(deviceName));

    if (!mSingleCommandHelper.packSingleCommand("StepGZJD_WorkMode_STTC", mPackCommand, deviceID, params))
    {
        auto msg = QString("%1设备组包失败: %2").arg(deviceName, mPackCommand.errorMsg);
        emit signalErrorMsg(msg);
        return false;
    }
    // 发送命令，不等待  判断是否发送成功
    return waitExecSuccess(mPackCommand, 0);
}

bool BaseHandler::switchJDWorkMode(const CKJDControlInfo& deviceInfo, SystemWorkMode needWorkMode, bool master)
{
    auto beforeDeviceID = master ? deviceInfo.beforeMaster : deviceInfo.beforeSlave;
    auto afterDeviceID = master ? deviceInfo.afterMaster : deviceInfo.afterSlave;
    // auto curModeID = master ? deviceInfo.curModeIDMaster : deviceInfo.curModeIDSlave;
    // auto beforeIsDouble = master ? deviceInfo.beforeMasterDoubleMode : deviceInfo.beforeSlaveDoubleMode;
    auto res = switchJDWorkModeNotWait(deviceInfo, needWorkMode, master);
    if (res == CKJDItemSwitchStatus::Success)
    {
        return true;
    }
    if (res == CKJDItemSwitchStatus::Failed)
    {
        return false;
    }

    auto deviceName = GlobalData::getExtensionName(beforeDeviceID);
    auto needWorkModeName = getCKJDWorkModeDesc(needWorkMode, deviceInfo.afterDoubleMode);
    auto needModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(needWorkMode);
    // 切换之后的设备ID如果是双模式  被设置成4001/4003 这里也把模式切换为S的
    needModeId = deviceInfo.afterDoubleMode ? SystemWorkModeHelper::systemWorkModeConverToModeId(Skuo2) : needModeId;

    emit signalInfoMsg(QString("正在切换%1模式为%2").arg(deviceName, needWorkModeName));

#if 0
    /* 这里要首先清除after的状态 否则直接会误读状态 */
    if (beforeIsDouble)
    {
        // 特别是双模式 他是一个一个退的 如果先清除状态 马上又会上报上来
        QThread::sleep(25);
    }
    else
    {
        QThread::sleep(5);
    }
#else
    QThread::sleep(5);
#endif

    GlobalData::setOnlineDeviceModeId(afterDeviceID, -1);
    auto tempDevice = getEqModeCKDeviceID(afterDeviceID);
    GlobalData::setOnlineDeviceModeId(tempDevice, -1);

    //  等待60s判断模式是否切换成功
    int waitTime = GlobalData::getCmdTimeCount(60);
    for (auto i = 0; i < waitTime; ++i)
    {
        ExitCheck(false);
        GlobalData::waitCmdTimeMS();
        // 如果模式切换成功就return
        auto curModeId = GlobalData::getOnlineDeviceModeId(afterDeviceID);
        if (curModeId == needModeId)
        {
            emit signalInfoMsg(QString("测控基带模式切换成功"));
            // 等待5s,测控基带再切换模式之后界面未显示之前就已经上报数据了
            // 等待3s 容错等另一台机器启动完成
            QThread::sleep(8);
            return true;
        }
    }

    emit signalInfoMsg(QString("测控基带模式切换超时"));
    return false;
}

bool BaseHandler::switchGZJDWorkMode(const DeviceID& deviceID, int needRealModeId)
{
    auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(deviceID))
    {
        emit signalErrorMsg(QString("%1设备离线").arg(GlobalData::getExtensionName(deviceID)));
        return false;
    }

    // 判断是否为本控
    if (!GlobalData::getDeviceSelfControl(deviceID))
    {
        emit signalErrorMsg(QString("%1不为本控").arg(GlobalData::getExtensionName(deviceID)));
        return false;
    }

    // 设备当前模式和需要的模式一致，不进行切换
    if (curModeId == needRealModeId)
    {
        return true;
    }

    // 发送工作模式切换命令

    QMap<QString, QVariant> paramMap;
    paramMap["WorkMode"] = needRealModeId;

    auto deviceName = GlobalData::getExtensionName(deviceID);

    emit signalInfoMsg(QString("%1正在切换模式").arg(deviceName));

    if (!mSingleCommandHelper.packSingleCommand("StepGZJD_WORK_MODE_SWITCH", mPackCommand, deviceID, paramMap))
    {
        return false;
    }
    // 发送命令，不等待  判断是否发送成功
    if (!waitExecSuccess(mPackCommand, 0))
    {
        return false;
    }

    //  等待60s判断模式是否切换成功
    int waitTime = GlobalData::getCmdTimeCount(60);
    for (auto i = 0; i < waitTime; ++i)
    {
        ExitCheck(false);
        GlobalData::waitCmdTimeMS();
        // 如果模式切换成功就return
        auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
        if (curModeId == needRealModeId)
        {
            emit signalInfoMsg(QString("跟踪基带模式切换成功"));
            QThread::sleep(3);  // 等待3s,等设备上报数据
            return true;
        }
    }

    return false;
}

BaseHandler::CKJDSwitchStatus BaseHandler::switchGZJDMasterAndSlaveWorkMode(const DeviceID& masterDeviceID, const DeviceID& slaveDeviceID,
                                                                            int needSwitchModeId)
{
    //先判断切换的模式是否相等
    auto deviceNameMaster = GlobalData::getExtensionName(masterDeviceID);
    auto deviceNameSlave = GlobalData::getExtensionName(slaveDeviceID);
    auto curModeIdMaster = GlobalData::getOnlineDeviceModeId(masterDeviceID);
    auto curModeIdSlave = GlobalData::getOnlineDeviceModeId(slaveDeviceID);
    // 设备当前模式和需要的模式一致，不进行切换
    if (curModeIdMaster == needSwitchModeId && curModeIdSlave == needSwitchModeId)
    {
        emit signalInfoMsg(QString("主机:%1切换成功").arg(deviceNameMaster));
        emit signalInfoMsg(QString("备机:%1切换成功").arg(deviceNameSlave));
        return CKJDSwitchStatus::All;
    }

    //暂用于切换同一个模式的主备机切换  暂不支持两组以上的主备机切换
    auto resMaster = switchGZJDWorkModeNotWait(masterDeviceID, needSwitchModeId);
    auto resSlave = switchGZJDWorkModeNotWait(slaveDeviceID, needSwitchModeId);

    //这里判断一下 如果主备机都没有切换成功就返回  只要有一个成功就继续下去，等待时间
    if (!resMaster && !resSlave)
    {
        return CKJDSwitchStatus::Failed;
    }

    //  等待60s判断模式是否切换成功
    int waitTime = GlobalData::getCmdTimeCount(60);
    bool successMaster = false;
    bool successSlave = false;
    for (auto i = 0; i < waitTime; ++i)
    {
        ExitCheck(CKJDSwitchStatus::Failed);
        GlobalData::waitCmdTimeMS();
        // 如果模式切换成功就return
        auto curModeIdMaster = GlobalData::getOnlineDeviceModeId(masterDeviceID);
        auto curModeIdSlave = GlobalData::getOnlineDeviceModeId(slaveDeviceID);
        if (curModeIdMaster == needSwitchModeId && !successMaster)
        {
            emit signalInfoMsg(QString("主机:%1切换成功").arg(deviceNameMaster));
            QThread::sleep(2);
            successMaster = true;
        }

        if (curModeIdSlave == needSwitchModeId && !successSlave)
        {
            emit signalInfoMsg(QString("备机:%1切换成功").arg(deviceNameSlave));
            successSlave = true;
        }

        if ((successMaster && resMaster) && (successSlave || !resSlave))
        {
            break;
        }
        else if ((successSlave && resSlave) && (successMaster || !resMaster))
        {
            break;
        }
    }
    //这里列举了所有可能的情况
    if (successMaster && successSlave)  //主备机都切换成功
    {
        return CKJDSwitchStatus::All;
    }
    else if (successMaster && !successSlave)  //主机切换成功 备机失败
    {
        return CKJDSwitchStatus::Master;
    }
    else if (!successMaster && successSlave)  //备机切换成功 主机失败
    {
        return CKJDSwitchStatus::Slave;
    }
    else  //主备机都切换失败
    {
        return CKJDSwitchStatus::Failed;
    }
}

DeviceID BaseHandler::getEqModeCKDeviceID(const DeviceID& deviceID)
{
    auto tempDeviceID = deviceID;
    if (tempDeviceID.extenID == 1)
    {
        tempDeviceID.extenID = 2;
    }
    else if (tempDeviceID.extenID == 2)
    {
        tempDeviceID.extenID = 1;
    }
    else if (tempDeviceID.extenID == 3)
    {
        tempDeviceID.extenID = 4;
    }
    else
    {
        tempDeviceID.extenID = 3;
    }
    return tempDeviceID;
}

// 获取当前设备ID对应另一台测控设备的ID
DeviceID BaseHandler::getNextCKDeviceID(const DeviceID& deviceID)
{
    auto tempDeviceID = deviceID;
    if (tempDeviceID.extenID == 1)
    {
        tempDeviceID.extenID = 3;
    }
    else if (tempDeviceID.extenID == 3)
    {
        tempDeviceID.extenID = 1;
    }
    else if (tempDeviceID.extenID == 2)
    {
        tempDeviceID.extenID = 4;
    }
    else
    {
        tempDeviceID.extenID = 2;
    }
    return tempDeviceID;
}
DeviceID BaseHandler::getChangeModeCKDeviceID(const DeviceID& deviceID, SystemWorkMode workMode)
{
    auto tempDeviceID = deviceID;
    if (workMode == Skuo2)
    {
        if (tempDeviceID.extenID == 2)
        {
            tempDeviceID.extenID = 1;
        }
        else if (tempDeviceID.extenID == 4)
        {
            tempDeviceID.extenID = 3;
        }
    }
    else if (workMode == KaKuo2)
    {
        if (tempDeviceID.extenID == 1)
        {
            tempDeviceID.extenID = 2;
        }
        else if (tempDeviceID.extenID == 3)
        {
            tempDeviceID.extenID = 4;
        }
    }
    return tempDeviceID;
}

bool BaseHandler::switchCKJDMasterSlave(const CKJDControlInfo& info)
{
    //     auto msg = QString("主机------%1--%2--%3").arg(info.beforeMaster.toHex(), info.afterMaster.toHex()).arg( info.curModeIDMaster);
    //     emit signalErrorMsg(msg);
    //     msg = QString("备机------%1--%2--%3").arg(info.beforeSlave.toHex(), info.afterSlave.toHex()).arg( info.curModeIDSlave);
    //     emit signalErrorMsg(msg);
    // 获取主用测控基带数量
    int masterNumber = 0;
    if (!getCKJDMasterNumber(masterNumber))
    {
        return false;
    }

    // 一主一备
    if (masterNumber == 1)
    {
        if (info.afterMaster.isValid())
        {
            switchJDMasterSlave(info.afterMaster, MasterSlave::Master);
        }

        if (info.afterSlave.isValid() && info.afterMaster != info.afterSlave)
        {
            switchJDMasterSlave(info.afterSlave, MasterSlave::Slave);
        }

        /* 双模式 切换4个 */
        if (info.afterDoubleMode)
        {
            if (info.afterMaster.isValid())
            {
                auto tempAfterMaster = getEqModeCKDeviceID(info.afterMaster);
                switchJDMasterSlave(tempAfterMaster, MasterSlave::Master);
            }
            if (info.afterSlave.isValid())
            {
                auto tempAfterSlave = getEqModeCKDeviceID(info.afterSlave);
                switchJDMasterSlave(tempAfterSlave, MasterSlave::Slave);
            }
        }
    }
    // 双主机
    else if (masterNumber == 2)
    {
        if (info.afterMaster.isValid())
        {
            switchJDMasterSlave(info.afterMaster, MasterSlave::Master);
        }
        if (info.afterSlave.isValid() && info.afterMaster != info.afterSlave)
        {
            switchJDMasterSlave(info.afterSlave, MasterSlave::Master);
        }
    }

    return true;
}

// bool BaseHandler::switchCKJDMasterSlave()
//{
//    // 获取主用测控基带数量
//    int masterNumber = 0;
//    if (!getCKJDMasterNumber(masterNumber))
//    {
//        return false;
//    }

//    DeviceID deviceID1;
//    DeviceID deviceID2;

//    getCKJDDeviceID(deviceID1, 1);
//    getCKJDDeviceID(deviceID2, 2);

//    // 一主一备
//    if (masterNumber == 1)
//    {
//        switchJDMasterSlave(deviceID1, MasterSlave::Master);
//        if (deviceID1 != deviceID2)
//        {
//            switchJDMasterSlave(deviceID2, MasterSlave::Slave);
//        }
//    }
//    // 双主机
//    else if (masterNumber == 2)
//    {
//        switchJDMasterSlave(deviceID1, MasterSlave::Master);
//        switchJDMasterSlave(deviceID2, MasterSlave::Master);
//    }

//    return true;
//}

bool BaseHandler::switchGSJDMasterSlave()
{
    auto result = getGSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }

    return switchGSJDMasterSlave(result.value());
}

bool BaseHandler::switchGSJDMasterSlave(const MasterSlaveDeviceIDInfo& info)
{
    // 获取主用高速基带数量
    int masterNumber = 0;
    if (!getGSJDMasterNumber(masterNumber))
    {
        return false;
    }

    // 一主一备
    if (masterNumber == 1)
    {
        if (info.master.isValid())
        {
            switchGSJDMasterSlave(info.master, MasterSlave::Master);
        }
        if (info.slave.isValid())
        {
            switchGSJDMasterSlave(info.slave, MasterSlave::Slave);
        }
    }
    // 两主
    else if (masterNumber == 2)
    {
        if (info.master.isValid())
        {
            switchGSJDMasterSlave(info.master, MasterSlave::Master);
        }
        if (info.slave.isValid())
        {
            switchGSJDMasterSlave(info.slave, MasterSlave::Master);
        }
    }

    return true;
}

bool BaseHandler::switchDSJDMasterSlave()
{
    auto result = getDSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }

    return switchDSJDMasterSlave(result.value());
}
bool BaseHandler::switchDSJDMasterSlave(const MasterSlaveDeviceIDInfo& info)
{
    // 获取主用低速基带数量
    int masterNumber = 0;
    if (!getDSJDMasterNumber(masterNumber))
    {
        return false;
    }

    // 一主一备
    if (masterNumber == 1)
    {
        if (info.master.isValid())
        {
            switchJDMasterSlave(info.master, MasterSlave::Master);
        }
        if (info.slave.isValid())
        {
            switchJDMasterSlave(info.slave, MasterSlave::Slave);
        }
    }
    // 两主
    else
    {
        if (info.master.isValid())
        {
            switchJDMasterSlave(info.master, MasterSlave::Master);
        }
        if (info.slave.isValid())
        {
            switchJDMasterSlave(info.slave, MasterSlave::Master);
        }
    }

    return true;
}
bool BaseHandler::switchGZJDMasterSlave()
{
    // 跟踪基带只对需要使用的基带设置为主用
    DeviceID gzjdDeviceID;
    if (!getMasterGZJDDeviceID(gzjdDeviceID))
    {
        return false;
    }

    switchJDMasterSlave(gzjdDeviceID, MasterSlave::Master);

    return true;
}

void BaseHandler::switchJDMasterSlave(const DeviceID& deviceID, MasterSlave masterSlave)
{
    auto extension = GlobalData::getExtension(deviceID);
    if (!extension.modeCtrlMap.isEmpty())
    {
        // 获取当前分机第一个模式Id
        auto firstModeId = extension.modeCtrlMap.firstKey();

        auto cmdId = (masterSlave == MasterSlave::Master ? "StepJDMaster" : "StepJDSlave");

        mSingleCommandHelper.packSingleCommand(cmdId, mPackCommand, deviceID, firstModeId);

        waitExecSuccess(mPackCommand);
    }
}

void BaseHandler::switchGSJDMasterSlave(const DeviceID& deviceID, MasterSlave masterSlave)
{
    auto extension = GlobalData::getExtension(deviceID);
    if (!extension.modeCtrlMap.isEmpty())
    {
        // 获取当前分机第一个模式Id
        auto firstModeId = extension.modeCtrlMap.firstKey();

        auto cmdId = (masterSlave == MasterSlave::Master ? "StepGSJDMaster" : "StepGSJDSlave");

        mSingleCommandHelper.packSingleCommand(cmdId, mPackCommand, deviceID, firstModeId);

        waitExecSuccess(mPackCommand);
    }
}

void BaseHandler::setRunningFlag(std::atomic<bool>* runningFlag) { mRunningFlag = runningFlag; }
bool BaseHandler::isRunning()
{
    if (mRunningFlag == nullptr)
    {
        return true;
    }
    return mRunningFlag->load();
}
bool BaseHandler::isExit()
{
    if (mRunningFlag == nullptr)
    {
        return false;
    }
    return !mRunningFlag->load();
}

Optional<QString> BaseHandler::getMasterTrackingTaskCode(const ManualMessage& mManualMsg)
{
    using ResType = Optional<QString>;
    QMap<SystemWorkMode, LinkLine> tempMaster;
    for (auto& link : mManualMsg.linkLineMap)
    {
        if (link.masterTargetNo <= 0 || !link.targetMap.contains(link.masterTargetNo))
        {
            continue;
        }
        tempMaster.insert(link.workMode, link);
    }
    if (tempMaster.isEmpty())
    {
        return ResType(ErrorCode::NotFound, "未找到当前主跟目标");
    }
    QSet<QString> taskCode;
    for (auto& item : tempMaster)
    {
        auto targetInfo = item.targetMap.value(item.masterTargetNo);
        if (!targetInfo.taskCode.isEmpty())
        {
            taskCode << targetInfo.taskCode;
        }
    }
    if (taskCode.isEmpty())
    {
        return ResType(ErrorCode::NotFound, "未找到当前主跟目标");
    }
    if (taskCode.size() >= 2)
    {
        auto tempDesc = QString("当前主跟目标数量过多：%1").arg(taskCode.toList().join("、"));
        return ResType(ErrorCode::NotFound, tempDesc);
    }

    return ResType(*taskCode.begin());
}
QList<SystemWorkMode> BaseHandler::trackingToSystemWorkModeList(SatelliteTrackingMode mode)
{
    using Restype = QList<SystemWorkMode>;
    if (mode == _4426_STTC)
    {
        return Restype{ STTC };
    }

    if (mode == _4426_SK2)
    {
        return Restype{ Skuo2 };
    }

    if (mode == _4426_SKT)
    {
        return Restype{ SKT };
    }

    if (mode == _4426_KaCk)
    {
        return Restype{ KaKuo2 };
    }

    if (mode == _4426_KaDSDT)
    {
        return Restype{ KaDS };
    }

    if (mode == _4426_KaGSDT)
    {
        return Restype{ KaGS };
    }

    if (mode == _4426_STTC_KaCk)
    {
        return Restype{ STTC, KaKuo2 };
    }

    if (mode == _4426_SK2_KaCk)
    {
        return Restype{ Skuo2, KaKuo2 };
    }

    if (mode == _4426_STTC_KaDSDT)
    {
        return Restype{ STTC, KaDS };
    }

    if (mode == _4426_SK2_KaDSDT)
    {
        return Restype{ Skuo2, KaDS };
    }

    if (mode == _4426_STTC_KaGSDT)
    {
        return Restype{ STTC, KaGS };
    }

    if (mode == _4426_SK2_KaGSDT)
    {
        return Restype{ Skuo2, KaGS };
    }

    if (mode == _4426_STTC_KaCk_KaDSDT)
    {
        return Restype{ STTC, KaKuo2, KaDS };
    }

    if (mode == _4426_SK2_KaCk_KaDSDT)
    {
        return Restype{ Skuo2, KaKuo2, KaDS };
    }

    if (mode == _4426_STTC_KaCK_KaGSDT)
    {
        return Restype{ STTC, KaKuo2, KaGS };
    }

    if (mode == _4426_SK2_KaCK_KaGSDT)
    {
        return Restype{ Skuo2, KaKuo2, KaGS };
    }

    return Restype();
}

SystemWorkMode BaseHandler::trackingToSystemWorkMode(SatelliteTrackingMode mode)
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
Optional<int> BaseHandler::getACUSystemWorkMode(SatelliteTrackingMode mode)
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
Optional<DeviceID> BaseHandler::getOnlineACU()
{
    using ResType = Optional<DeviceID>;
    DeviceID acuADeviceID(ACU_A);
    DeviceID acuBDeviceID(ACU_B);

    if (!GlobalData::getDeviceOnline(acuADeviceID) && !GlobalData::getDeviceOnline(acuBDeviceID))
    {
        return ResType(ErrorCode::DevOffline, "ACU主备设备离线");
    }

    /*
     * 1 在线 表示当前主机
     * 2 离线 表示当前备机
     * 根据4413的情况ACU的主备是绝对的主备 不能两个同时使用的
     */
    auto tempVarA = GlobalData::getReportParamData(acuADeviceID, 0x01, "OnLineMachine");
    auto tempVarB = GlobalData::getReportParamData(acuBDeviceID, 0x01, "OnLineMachine");
    if (tempVarA.toInt() == 1)
    {
        return ResType(acuADeviceID);
    }
    else if (tempVarB.toInt() == 1)
    {
        return ResType(acuBDeviceID);
    }

    return ResType(ErrorCode::InvalidData, "未找到当前在线ACU");
}
void BaseHandler::cleanDTETask()
{
    if (!mManualMsg.resourceReleaseDeleteDTETask)
    {
        return;
    }
    auto staticUACResult = getStationCKUAC();
    if (!staticUACResult)
    {
        emit signalErrorMsg(staticUACResult.msg());
        return;
    }

    /* 先获取DTE上报数据 获取任务信息 */
    auto stepKey = QString("Step_DTE_TASK_DELETE");
    auto deviceID = mSingleCommandHelper.getSingleCmdDeviceID(stepKey, "0x8001");
    auto dteInfo = GlobalData::getExtenStatusReportData(deviceID);

    auto recycleReportMap = dteInfo.unitReportMsgMap.value(1).multiTargetParamMap2;  // 只有一个单元
    auto missionReportMap = recycleReportMap.value(1);

    QVariantMap replaceMap;
    replaceMap["UAC"] = staticUACResult.value();
    for (auto& item : missionReportMap)
    {
        replaceMap["MID"] = item.value("MissionID");
        mSingleCommandHelper.packSingleCommand(stepKey, mPackCommand, replaceMap);
        waitExecSuccess(mPackCommand);
    }
}

/* 获取当前的站ID */
Optional<QString> BaseHandler::getStationCKUAC()
{
    using ResType = Optional<QString>;
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
    return ResType(info.ttcUACAddr);
}
