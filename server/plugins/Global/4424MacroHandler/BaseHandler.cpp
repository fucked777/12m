#include "BaseHandler.h"

#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "RedisHelper.h"
#include "ResourceRestructuringMessageDefine.h"
#include "SatelliteManagementDefine.h"
#include "SubscriberHelper.h"

#include <QThread>

BaseHandler::BaseHandler(QObject* parent)
    : QObject(parent)
    , mRunningFlag(nullptr)
{
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalControlCmdResponseReadable, this, &BaseHandler::slotControlCmdResponse);
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

bool BaseHandler::getGSJDDeviceID(DeviceID& deviceID, int no)
{
    mManualMsg.configMacroData.getGSJDDeviceID(mLinkLine.workMode, deviceID, no);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取高速基带失败");
        return false;
    }
    return true;
}

void BaseHandler::getGSJDDeviceIDS(QList<DeviceID>& deviceIDList)
{
    auto curProject = ExtendedConfig::curProjectID();
    if (curProject == "4424")
    {
        DeviceID deviceID1;
        DeviceID deviceID2;
        DeviceID deviceID3;

        if (getGSJDDeviceID(deviceID1, 1))
        {
            deviceIDList.append(deviceID1);
        }
        if (getGSJDDeviceID(deviceID2, 2))
        {
            deviceIDList.append(deviceID2);
        }
        if (getGSJDDeviceID(deviceID3, 3))
        {
            deviceIDList.append(deviceID3);
        }
    }
    else if (curProject == "4426")
    {
        DeviceID deviceID1;
        DeviceID deviceID2;

        if (getGSJDDeviceID(deviceID1, 1))
        {
            deviceIDList.append(deviceID1);
        }
        if (getGSJDDeviceID(deviceID2, 2))
        {
            deviceIDList.append(deviceID2);
        }
    }
}

bool BaseHandler::getDSJDDeviceID(DeviceID& deviceID, int no)
{
    mManualMsg.configMacroData.getDSJDDeviceID(mLinkLine.workMode, deviceID, no);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取低速基带失败");
        return false;
    }
    return true;
}

void BaseHandler::getDSJDDeviceIDS(QList<DeviceID>& deviceIDList)
{
    DeviceID deviceID1;
    DeviceID deviceID2;

    if (getDSJDDeviceID(deviceID1, 1))
    {
        deviceIDList.append(deviceID1);
    }
    if (getDSJDDeviceID(deviceID2, 2))
    {
        deviceIDList.append(deviceID2);
    }
}

bool BaseHandler::getCKQDDeviceID(DeviceID& deviceID)
{
    mManualMsg.configMacroData.getCKQDDeviceID(mLinkLine.workMode, deviceID);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取测控数字化前端失败");
        return false;
    }

    return true;
}

bool BaseHandler::getGZQDDeviceID(DeviceID& deviceID)
{
    mManualMsg.configMacroData.getGZQDDeviceID(mLinkLine.workMode, deviceID);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取跟踪数字化前端失败");
        return false;
    }
    return true;
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

bool BaseHandler::getXXBPQDeviceID(DeviceID& deviceID, int no)
{
    mManualMsg.configMacroData.getXXBPQDeviceID(mLinkLine.workMode, deviceID, no);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取X1.2GHz下变频器失败");
        return false;
    }
    return true;
}

bool BaseHandler::getKaXBPQDeviceID(DeviceID& deviceID, int no)
{
    mManualMsg.configMacroData.getKaXBPQDeviceID(mLinkLine.workMode, deviceID, no);
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取Ka1.2GHz下变频器失败");
        return false;
    }
    return true;
}

bool BaseHandler::getKaGSXBPQDeviceID(DeviceID& deviceID, int& unitId, int no)
{
    auto key = QString("DC%1").arg(no);

    mManualMsg.configMacroData.getDeviceID(mLinkLine.workMode, deviceID, key, 1);

    if (!deviceID.isValid())
    {
        emit signalErrorMsg("获取Ka高速下变频器失败");
        return false;
    }

    QVariant value;
    mManualMsg.configMacroData.getValue(mLinkLine.workMode, key, value);
    if (!value.isValid())
    {
        emit signalErrorMsg("获取Ka高速下变频器失败");
        return false;
    }

    unitId = value.toInt() + 2;

    return true;
}

bool BaseHandler::getMasterGZJDDeviceID(DeviceID& deviceID)
{
    // 跟踪前端和跟踪基带一对一
    DeviceID gzqdDeviceID;
    if (!getGZQDDeviceID(gzqdDeviceID))
    {
        emit signalErrorMsg("获取跟踪基带失败");
        return false;
    }

    deviceID = DeviceID(4, 2, gzqdDeviceID.extenID);
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
    if (isExit())
    {
        return false;
    }

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
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        auto msg = QString("%1设备离线，发送%2命令%3失败")
                       .arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(),
                            currentCmdOperatorInfo.isEmpty() ? "" : QString("(%1)").arg(currentCmdOperatorInfo));
        emit signalErrorMsg(msg);
        return false;
    }

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
    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    while (ttl > 0)
    {
        if (isExit())
        {
            return false;
        }
        QThread::sleep(1);
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

void BaseHandler::appendExecQueue(const PackCommand& packCommand) { mPackCommandQueue << packCommand; }

void BaseHandler::execQueue(int ttl)
{
    for (const auto& packCommand : mPackCommandQueue)
    {
        if (isExit())
        {
            return;
        }

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
        auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;

        // 判断设备是否在线
        if (!GlobalData::getDeviceOnline(packCommand.deviceID))
        {
            auto msg = QString("%1设备离线，发送%2命令%3失败")
                           .arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(),
                                currentCmdOperatorInfo.isEmpty() ? "" : QString("(%1)").arg(currentCmdOperatorInfo));
            emit signalErrorMsg(msg);
        }

        // 发送数据
        emit signalSendToDevice(packCommand.data);

        // 记录发送的命令
        mSentCommandList << packCommand;

        // 写入日志当前命令执行的操作(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
        currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
        emit signalInfoMsg(currentCmdOperatorInfo);
    }

    // 没有发送的命令直接返回
    if (mSentCommandList.isEmpty())
    {
        return;
    }

    // 等待超时
    while (ttl > 0)
    {
        if (isExit())
        {
            return;
        }
        QThread::sleep(1);
        --ttl;
    }

    // 如果超过最大等待时间还没有响应就判断该命令执行超时
    for (const auto& sentCmd : mSentCommandList)
    {
        if (isExit())
        {
            return;
        }

        // 写入日志当前命令执行的操作(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
        auto currentCmdOperatorInfo = sentCmd.operatorInfo.isEmpty() ? sentCmd.getSetParamDescValue() : sentCmd.operatorInfo;
        currentCmdOperatorInfo = QString("%1-%2: %3").arg(sentCmd.getCmdDeviceName(), sentCmd.getCmdName(), currentCmdOperatorInfo);
        // 超时未响应
        emit signalErrorMsg(QString("%1,超时未响应").arg(currentCmdOperatorInfo));
    }
}

bool BaseHandler::switchJDWorkMode(const DeviceID& deviceID, SystemWorkMode needWorkMode)
{
    auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
    int needModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(needWorkMode);

    // 设备当前模式和需要的模式一致，不进行切换
    if (curModeId == needModeId)
    {
        return true;
    }
    auto deviceName = GlobalData::getExtensionName(deviceID);
    auto needWorkModeName = SystemWorkModeHelper::systemWorkModeToDesc(needWorkMode);

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(deviceID))
    {
        emit signalErrorMsg(QString("%1设备不在线").arg(GlobalData::getExtensionName(deviceID)));
        return false;
    }

    // 判断是否为本控
    if (!GlobalData::getDeviceSelfControl(deviceID))
    {
        emit signalErrorMsg(QString("%1不为本控").arg(GlobalData::getExtensionName(deviceID)));
        return false;
    }

    // 发送工作模式切换命令
    SingleCommandHelper singleCommandHelper;
    QString singleCmdId = QString("StepJD_WORK_MODE_SWITCH_%1").arg(needWorkMode);
    if (!singleCommandHelper.packSingleCommand(singleCmdId, mPackCommand, deviceID))
    {
        return false;
    }
    // 发送命令，不等待  判断是否发送成功
    if (!waitExecSuccess(mPackCommand, 0))
    {
        return false;
    }

    emit signalInfoMsg(QString("正在切换%1模式为%2").arg(deviceName, needWorkModeName));

    //  等待60s判断模式是否切换成功
    int waitTime = 60;
    for (auto i = 0; i < waitTime; ++i)
    {
        if (isExit())
        {
            return false;
        }
        QThread::sleep(1);
        // 如果模式切换成功就return
        auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
        if (curModeId == needModeId)
        {
            emit signalInfoMsg(QString("模式切换成功"));
            QThread::sleep(3);  // 等待3s,等设备上报数据
            return true;
        }
    }

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
    int waitTime = 60;
    for (auto i = 0; i < waitTime; ++i)
    {
        if (isExit())
        {
            return false;
        }
        QThread::sleep(1);
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

bool BaseHandler::switchCKJDMasterSlave(const CKJDControlInfo& info)
{
    // 获取主用测控基带数量
    int masterNumber = 0;
    if (!getCKJDMasterNumber(masterNumber))
    {
        return false;
    }

    // 一主一备
    if (masterNumber == 1)
    {
        switchJDMasterSlave(info.afterMaster, MasterSlave::Master);
        if (info.afterMaster != info.afterSlave)
        {
            switchJDMasterSlave(info.afterSlave, MasterSlave::Slave);
        }
    }
    // 双主机
    else if (masterNumber == 2)
    {
        switchJDMasterSlave(info.afterMaster, MasterSlave::Master);
        switchJDMasterSlave(info.afterSlave, MasterSlave::Master);
    }

    return true;
}

bool BaseHandler::switchCKJDMasterSlave()
{
    // 获取主用测控基带数量
    int masterNumber = 0;
    if (!getCKJDMasterNumber(masterNumber))
    {
        return false;
    }

    DeviceID deviceID1;
    DeviceID deviceID2;

    getCKJDDeviceID(deviceID1, 1);
    getCKJDDeviceID(deviceID2, 2);

    // 一主一备
    if (masterNumber == 1)
    {
        switchJDMasterSlave(deviceID1, MasterSlave::Master);
        if (deviceID1 != deviceID2)
        {
            switchJDMasterSlave(deviceID2, MasterSlave::Slave);
        }
    }
    // 双主机
    else if (masterNumber == 2)
    {
        switchJDMasterSlave(deviceID1, MasterSlave::Master);
        switchJDMasterSlave(deviceID2, MasterSlave::Master);
    }

    return true;
}

bool BaseHandler::switchGSJDMasterSlave()
{
    // 获取主用高速基带数量
    int masterNumber = 0;
    if (!getGSJDMasterNumber(masterNumber))
    {
        return false;
    }

    auto curProject = ExtendedConfig::curProjectID();
    if (curProject == "4424")
    {
        DeviceID deviceID1;
        DeviceID deviceID2;
        DeviceID deviceID3;
        getGSJDDeviceID(deviceID1, 1);
        getGSJDDeviceID(deviceID2, 2);
        getGSJDDeviceID(deviceID3, 3);

        // 一主两备
        if (masterNumber == 1)
        {
            switchJDMasterSlave(deviceID1, MasterSlave::Master);
            switchJDMasterSlave(deviceID2, MasterSlave::Slave);
            switchJDMasterSlave(deviceID3, MasterSlave::Slave);
        }
        // 两主一备
        else if (masterNumber == 2)
        {
            switchJDMasterSlave(deviceID1, MasterSlave::Master);
            switchJDMasterSlave(deviceID2, MasterSlave::Master);
            switchJDMasterSlave(deviceID3, MasterSlave::Slave);
        }
    }
    else if (curProject == "4426")
    {
        DeviceID deviceID1;
        DeviceID deviceID2;
        getGSJDDeviceID(deviceID1, 1);
        getGSJDDeviceID(deviceID2, 2);

        // 一主一备
        if (masterNumber == 1)
        {
            switchGSJDMasterSlave(deviceID1, MasterSlave::Master);
            if (deviceID1 != deviceID2)
            {
                switchGSJDMasterSlave(deviceID2, MasterSlave::Slave);
            }
        }
        // 两主
        else if (masterNumber == 2)
        {
            switchGSJDMasterSlave(deviceID1, MasterSlave::Master);
            switchGSJDMasterSlave(deviceID2, MasterSlave::Master);
        }
    }

    return true;
}

bool BaseHandler::switchDSJDMasterSlave()
{
    // 获取主用低速基带数量
    int masterNumber = 0;
    if (!getDSJDMasterNumber(masterNumber))
    {
        return false;
    }

    DeviceID deviceID1;
    DeviceID deviceID2;

    getDSJDDeviceID(deviceID1, 1);
    getDSJDDeviceID(deviceID2, 2);

    // 一主一备
    if (masterNumber == 1)
    {
        switchJDMasterSlave(deviceID1, MasterSlave::Master);
        switchJDMasterSlave(deviceID2, MasterSlave::Slave);
    }
    // 两主
    else
    {
        switchJDMasterSlave(deviceID1, MasterSlave::Master);
        switchJDMasterSlave(deviceID2, MasterSlave::Master);
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

void BaseHandler::slotControlCmdResponse(const ControlCmdResponse& cmdResponse)
{
    for (const auto& sentCmd : mSentCommandList)
    {
        if (isExit())
        {
            return;
        }
        // 判断该控制命令响应是否是本对象的命令响应
        if (sentCmd.cmdType == cmdResponse.cmdType && sentCmd.deviceID == cmdResponse.deviceID && sentCmd.modeId == cmdResponse.modeId &&
            sentCmd.cmdId == cmdResponse.cmdId)
        {
            // 写入日志当前命令执行的操作(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
            auto currentCmdOperatorInfo = sentCmd.operatorInfo.isEmpty() ? sentCmd.getSetParamDescValue() : sentCmd.operatorInfo;
            currentCmdOperatorInfo = QString("%1-%2: %3").arg(sentCmd.getCmdDeviceName(), sentCmd.getCmdName(), currentCmdOperatorInfo);

            // 发送响应日志
            auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponse.responseResult);
            QString message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);
            switch (cmdResponse.responseResult)
            {
            case ControlCmdResponseType::Success: emit signalInfoMsg(message); break;
            case ControlCmdResponseType::SubControlRejected: emit signalWarningMsg(message); break;
            default: emit signalErrorMsg(message); break;
            }

            // 移除当前响应的命令
            mSentCommandList.removeOne(sentCmd);

            break;
        }
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
