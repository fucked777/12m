#include "DeviceProcessInfo.h"

#include "CConverter.h"
#include "ControlCmdResponseUnpacker.h"
#include "ControlResultReportUnpacker.h"
#include "CppMicroServicesUtility.h"
#include "CustomPacketMessageSerialize.h"
#include "DevProtocol.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "ExtensionStatusReportUnpacker.h"
#include "GlobalData.h"
#include "GroupParamSetPacker.h"
#include "LocalCommunicationAddr.h"
#include "MessagePublish.h"
#include "PackDefine.h"
#include "PacketHandler.h"
#include "ParameterHelper.h"
#include "ProcessControlCmdPacker.h"
#include "RedisHelper.h"
#include "StatusPersistenceMessageSerialize.h"
#include "StatusQueryCmdPacker.h"
#include "UnitParamSetPacker.h"
#include "Utility.h"
#include <QDateTime>
#include <QtConcurrent>

class DeviceProcessInfoImpl
{
public:
    struct MessageAddrMapStruct
    {
        qint64 lastUpdateTime{ 0 }; /* 最后一次更新的时间 */
        ProtocolPack pack;
        // QMap<QString, std::tuple<ProtocolPack, int>> mapDesAddr; /* 映射客户端地址,以及次数 */
        // QMap<QString, std::tuple<ProtocolPack, qint64>> mapDesAddr; /* 映射客户端地址,以及最后一次更新的时间 */
    };

    QMap<QString, QMap<QString, MessageAddrMapStruct>> cmdSrcCache; /* 这里做一个缓存,谁发的向谁推送 */

    void recordingMessageSrc(const MessageAddress& addr, quint32 cmdOrUnitID, const ProtocolPack& pack)
    {
        auto key = DeviceProcessHelper::getCmdResponseRedisKey(addr, cmdOrUnitID);
        auto& tempStruct = cmdSrcCache[key];
        auto curMS = GlobalData::currentDateTime().toMSecsSinceEpoch();
        for (auto iter = tempStruct.begin(); iter != tempStruct.end();)
        {
            /* 最后一次更新的时间，认为很久都没有用了,直接删除 */
            if (qAbs(curMS - iter->lastUpdateTime) > 60 * 1000)
            {
                iter = tempStruct.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        MessageAddrMapStruct tempData;
        tempData.lastUpdateTime = GlobalData::currentDateTime().toMSecsSinceEpoch();
        tempData.pack = pack;
        tempStruct[pack.desID] = tempData;
    }
};
DeviceProcessInfo::DeviceProcessInfo(QObject* parent)
    : QObject(parent)
    , m_impl(new DeviceProcessInfoImpl)
{
    connect(this, &DeviceProcessInfo::sg_acceptDeviceMessage, this, &DeviceProcessInfo::acceptDeviceMessage);
    connect(this, &DeviceProcessInfo::sg_acceptClientUnitSettingMessage, this, &DeviceProcessInfo::acceptClientUnitSettingMessage);
    connect(this, &DeviceProcessInfo::sg_acceptClientCmdSettingMessage, this, &DeviceProcessInfo::acceptClientCmdSettingMessage);
    connect(this, &DeviceProcessInfo::sg_acceptClientParamMacroSettingMessage, this, &DeviceProcessInfo::acceptClientParamMacroSettingMessage);
}

DeviceProcessInfo::~DeviceProcessInfo() {}

void DeviceProcessInfo::acceptDeviceMessage(const ProtocolPack& pack)
{
    UnpackMessage unpackMsg;
    bool flag = PacketHandler::instance().unpack(pack.data, unpackMsg);

    if (!flag)
    {
        auto systemName = GlobalData::getSystemName(unpackMsg.header.sourceAddr.systemNumb);
        auto deviceName = GlobalData::getDeviceName(unpackMsg.header.sourceAddr.systemNumb, unpackMsg.header.sourceAddr.deviceNumb);
        auto extenName = GlobalData::getExtensionName(unpackMsg.header.sourceAddr);
        auto msgType = DevProtocolEnumHelper::devMsgTypeToDescStr(unpackMsg.header.msgType);

        qWarning() << "解析设备数据出错:" << QString("%1-%2-%3-%4").arg(systemName, deviceName, extenName, msgType);
        return;
    }
    // 如果消息是分机状态上报，就将这个消息解析写入redis
    if (unpackMsg.header.msgType == DevMsgType::ExtensionStatusReport)
    {
        extensionStatusReport(unpackMsg);
    }
    // 控制命令响应
    else if (unpackMsg.header.msgType == DevMsgType::ControlCmdResponse)
    {
        controlCmdResponse(unpackMsg);
    }
    // 控制结果上报
    else if (unpackMsg.header.msgType == DevMsgType::ControlResultReport)
    {
        controlResultReport(unpackMsg);
    }
}

void DeviceProcessInfo::extensionStatusReport(const UnpackMessage& unpackMsg)
{
    const auto& sourceAddr = unpackMsg.header.sourceAddr;

    // 保存分机状态上报数据
    GlobalData::setExtenStatusReportData(sourceAddr.systemNumb, sourceAddr.deviceNumb, sourceAddr.extenNumb, unpackMsg.extenStatusReportMsg);

    // 保存分机模式
    GlobalData::setOnlineDeviceModeId(sourceAddr.systemNumb, sourceAddr.deviceNumb, sourceAddr.extenNumb, unpackMsg.extenStatusReportMsg.modeId);

    // 设置设备在线状态
    GlobalData::setDeviceOnline(sourceAddr.systemNumb, sourceAddr.deviceNumb, sourceAddr.extenNumb);

    // 发送状态到健康管理模块
    sendStatusToHealthManager(unpackMsg);
}

void DeviceProcessInfo::controlCmdResponse(const UnpackMessage& unpackMsg)
{
    auto controlCmdRespMsg = unpackMsg.controlCmdRespMsg;

    auto addr = unpackMsg.header.sourceAddr;
    int currentModeId = GlobalData::getOnlineDeviceModeId(addr.systemNumb, addr.deviceNumb, addr.extenNumb);
#if 0
    auto extenName = GlobalData::getExtensionName(unpackMsg.header.sourceAddr);
    auto msgType = DevProtocolEnumHelper::devMsgTypeToDescStr(controlCmdRespMsg.msgType);
    auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(controlCmdRespMsg.result);

    QString cmdName;
    if (controlCmdRespMsg.msgType == DevMsgType::UnitParameterSetCmd)
    {
        cmdName = GlobalData::getUnitName(addr, currentModeId, controlCmdRespMsg.cmdId);
    }
    else if (controlCmdRespMsg.msgType == DevMsgType::ProcessControlCmd)
    {
        cmdName = GlobalData::getCmdName(addr, currentModeId, controlCmdRespMsg.cmdId);
    }
    else if (controlCmdRespMsg.msgType == DevMsgType::GroupParamSetCmd)
    {
        cmdName = QString("组参数设置");
    }
    qDebug() << QString("%1-%2-%3(%4)-%5").arg(extenName).arg(msgType).arg(cmdName).arg(controlCmdRespMsg.cmdId).arg(result);
#endif
    ControlCmdResponse cmdResponce;
    cmdResponce.deviceID =
        DeviceID(unpackMsg.header.sourceAddr.systemNumb, unpackMsg.header.sourceAddr.deviceNumb, unpackMsg.header.sourceAddr.extenNumb);
    cmdResponce.modeId = currentModeId;
    cmdResponce.cmdId = controlCmdRespMsg.cmdId;
    cmdResponce.cmdType = controlCmdRespMsg.msgType;
    cmdResponce.responseResult = controlCmdRespMsg.result;
    auto cacheKey = DeviceProcessHelper::getCmdResponseRedisKey(cmdResponce.deviceID, cmdResponce.cmdId);

    /************************** 推送命令响应报文给客户端 **************************/
    /* 这个时间很短5秒足够 */
    /*
     * 这里采取特殊处理
     * 一般发送的命令是发送和接收是一一对应的
     * 但是这个命令是例外
     * 他是一次发送,最多有两种上报
     * 第一种是命令响应,这种很快一般在5秒以内就会响应,只会回复一次 单元和过程 组参数
     * 第二种是命令结果这种只有过程命令有,而且不是一定有结果的 且结果可以多次返回
     *
     * 根据以上情况
     * 接口如下
     * 命令响应，会根据当前设备ID命令或单元ID存入redis,同时会向发送方以固定的名称AcceptDeviceCMDResponceMessage返回ack调用
     * 结果上报,只会向发送方以固定的名称AcceptDeviceCMDResultMessage返回ack调用
     *
     * 响应可以查redis也可以定义函数接收
     * 结果只能定义函数接收
     */
    QByteArray tempPushData;
    tempPushData << cmdResponce;
    cmdResponcePush(cacheKey, "AcceptDeviceCMDResponceMessage", tempPushData, 10 * 1000);
    // 接收到响应，写入到redis中，5s的ttl超时时间
    RedisHelper::getInstance().setData(cacheKey, QString(tempPushData), 5);

    // 推送系统日志和保存日志(判断该命令的响应是否是通过本模块发送出去的，不是不进行推送，避免重复推送)
    auto find = m_impl->cmdSrcCache.find(cacheKey);
    if (find == m_impl->cmdSrcCache.end())
    {
        return;
    }
    // 删除当前命令缓存
    // 这有个问题，如果设备先上报响应，在这里移除掉了key，那么在控制结果上报的时候就找不到这个key，就无法将结果推送给客户端，暂时注释掉
    //  m_impl->cmdSrcCache.remove(cacheKey);

    auto extensionName = GlobalData::getExtensionName(cmdResponce.deviceID);
    QString unitOrCmdName;
    if (cmdResponce.cmdType == DevMsgType::UnitParameterSetCmd)
    {
        unitOrCmdName = GlobalData::getUnitName(cmdResponce.deviceID, cmdResponce.modeId, cmdResponce.cmdId);
    }
    else if (cmdResponce.cmdType == DevMsgType::ProcessControlCmd)
    {
        unitOrCmdName = GlobalData::getCmdName(cmdResponce.deviceID, cmdResponce.modeId, cmdResponce.cmdId);
    }
    else
    {
        return;
    }

    auto cmdTypeDesc = DevProtocolEnumHelper::devMsgTypeToDescStr(cmdResponce.cmdType);
    auto responceDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);

    auto logMsg = QString("%1：%2 %3 (%4)").arg(extensionName, unitOrCmdName, cmdTypeDesc, responceDesc);
    switch (cmdResponce.responseResult)
    {
    case ControlCmdResponseType::Success: SystemLogPublish::infoMsg(logMsg); break;
    case ControlCmdResponseType::SubControlRejected: SystemLogPublish::warningMsg(logMsg); break;
    default: SystemLogPublish::errorMsg(logMsg); break;
    }
}

void DeviceProcessInfo::controlResultReport(const UnpackMessage& unpackMsg)
{
    int modeId = GlobalData::getOnlineDeviceModeId(unpackMsg.header.sourceAddr.systemNumb, unpackMsg.header.sourceAddr.deviceNumb,
                                                   unpackMsg.header.sourceAddr.extenNumb);

    CmdResult result;
    result.m_systemNumb = unpackMsg.header.sourceAddr.systemNumb;
    result.m_deviceNumb = unpackMsg.header.sourceAddr.deviceNumb;
    result.m_extenNumb = unpackMsg.header.sourceAddr.extenNumb;
    result.m_paramdataMap = unpackMsg.controlResultReportMsg.paramMap;
    result.m_multiParamMap = unpackMsg.controlResultReportMsg.multiParamMap;
    result.m_modeID = modeId;
    result.m_cmdID = unpackMsg.controlResultReportMsg.id;

    /************************** 推送过程命令响应报文给客户端 **************************/
    /* 这种方式不可靠,但是木有办法,因为设备协议没有附带参数,能识别这个东东,有一个索引但是我们没判 */
    /* 结果的预设值是600秒,因为有些数据的上报结果会等很久的 */
    /*
     * 这里采取特殊处理
     * 一般发送的命令是发送和接收是一一对应的
     * 但是这个命令是例外
     * 他是一次发送,最多有两种上报
     * 第一种是命令响应,这种很快一般在5秒以内就会响应,只会回复一次 单元和过程 组参数
     * 第二种是命令结果这种只有过程命令有,而且不是一定有结果的 且结果可以多次返回
     *
     * 根据以上情况
     * 接口如下
     * 命令响应，会根据当前设备ID命令或单元ID存入redis,同时会向发送方以固定的名称AcceptDeviceCMDResponceMessage返回ack调用
     * 结果上报,只会向发送方以固定的名称AcceptDeviceCMDResultMessage返回ack调用
     *
     * 响应可以查redis也可以定义函数接收
     * 结果只能定义函数接收
     */
    auto cacheKey = DeviceProcessHelper::getCmdResponseRedisKey(unpackMsg.header.sourceAddr, result.m_cmdID);
    QByteArray tempPushData;
    tempPushData << result;
    cmdResponcePush(cacheKey, "AcceptDeviceCMDResultMessage", tempPushData, 600 * 1000);

    /* 星历数据 */
    if (result.m_systemNumb == 4 && result.m_deviceNumb == 4 && result.m_cmdID == 3)
    {
        QMap<QString, QVariant> params;

        int count = 0;
        for (int i = 50; i < unpackMsg.rawData.size(); i += 8)
        {
            QVariant x, y;
            ParameterHelper::fromByteArray(unpackMsg.rawData.mid(i, 4), AttributeType_Float, x);
            ParameterHelper::fromByteArray(unpackMsg.rawData.mid(i + 4, 4), AttributeType_Float, y);
            params.insert(QString("x_%1").arg(count), x.toDouble());
            params.insert(QString("y_%1").arg(count++), y.toDouble());
        }
        params.insert("count", count);
        result.m_paramdataMap.unite(params);

        QString tempString;
        tempString << result;

        DeviceID id(result.m_systemNumb, result.m_deviceNumb, result.m_extenNumb);
        int iDeviceId = 0;
        id >> iDeviceId;
        QString channel = result.m_paramdataMap.value("Channel").toString();
        RedisHelper::getInstance().setData(QString("StarMap_%1_%2").arg(iDeviceId).arg(channel), tempString, 5);
    }
    else if (result.m_systemNumb == 4 && result.m_deviceNumb == 1 && result.m_cmdID == 6)
    {
        QMap<QString, QVariant> params;

        int count = 0;
        for (int i = 46; i < unpackMsg.rawData.size(); i += 8)
        {
            QVariant x, y;
            ParameterHelper::fromByteArray(unpackMsg.rawData.mid(i, 4), AttributeType_Float, x);
            ParameterHelper::fromByteArray(unpackMsg.rawData.mid(i + 4, 4), AttributeType_Float, y);
            params.insert(QString("x_%1").arg(count), x.toDouble());
            params.insert(QString("y_%1").arg(count++), y.toDouble());
        }
        params.insert("count", count);
        result.m_paramdataMap.unite(params);

        QString tempString;
        tempString << result;

        DeviceID id(result.m_systemNumb, result.m_deviceNumb, result.m_extenNumb);
        int iDeviceId = 0;
        id >> iDeviceId;
        QString channel = result.m_paramdataMap.value("Channel").toString();
        RedisHelper::getInstance().setData(QString("StarMap_%1_%2").arg(iDeviceId).arg(channel), tempString, 5);
    }
    else
    {
        cacheKey = DeviceProcessHelper::getCmdResultRedisKey(unpackMsg.header.sourceAddr, result.m_cmdID);
        // 接收命令结果，写入到redis中，5s的ttl超时时间
        RedisHelper::getInstance().setData(cacheKey, QString(tempPushData), 10);
    }
}
void DeviceProcessInfo::cmdResponcePush(const QString& cacheKey, const QString& ackString, const QByteArray& data, int timeMS)
{
    auto find = m_impl->cmdSrcCache.find(cacheKey);
    if (find == m_impl->cmdSrcCache.end())
    {
        return;
    }

    auto& tempStruct = find.value();
    auto curMS = GlobalData::currentDateTime().toMSecsSinceEpoch();

    for (auto iter = tempStruct.begin(); iter != tempStruct.end();)
    {
        if (qAbs(curMS - iter->lastUpdateTime) > timeMS)
        {
            iter = tempStruct.erase(iter);
        }
        else
        {
            auto& tempData = iter.value();
            tempData.pack.data = data;
            tempData.pack.operationACK = ackString;
            qWarning() << tempData.pack.desID << tempData.pack.srcID;
            emit sg_cmdACK(tempData.pack);

            ++iter;
        }
    }
}

void DeviceProcessInfo::acceptClientUnitSettingMessage(const ProtocolPack& pack, const UnitParamRequest& paramRequest)
{
    UnitParamSetMessage unitSetMsg;
    unitSetMsg.mode = paramRequest.m_modeID;
    unitSetMsg.unitId = paramRequest.m_unitID;
    unitSetMsg.channelValidIdent = paramRequest.m_validIdent;
    unitSetMsg.settingParamMap = paramRequest.m_paramdataMap;
    for (auto targetNo : paramRequest.m_multiTargetParamMap.keys())
    {
        unitSetMsg.multiTargetParamMap[targetNo] = paramRequest.m_multiTargetParamMap[targetNo].m_paramdataMap;
    }

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();

    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = paramRequest.m_systemNumb;
    targetAddr.deviceNumb = paramRequest.m_deviceNumb;
    targetAddr.extenNumb = paramRequest.m_extenNumb;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;

    PackMessage packMsg;
    packMsg.header.msgType = DevMsgType::UnitParameterSetCmd;
    packMsg.header.sourceAddr = sourceAddr;
    packMsg.header.targetAddr = targetAddr;
    packMsg.header.version = version;
    packMsg.unitParamSetMsg = unitSetMsg;

    // 组包和推送日志
    packAndPublishCmdInfo(packMsg, pack);
}

void DeviceProcessInfo::acceptClientCmdSettingMessage(const ProtocolPack& pack, const CmdRequest& cmdRequest)
{
    ProcessControlCmdMessage controlCtrlCmdMsg;
    controlCtrlCmdMsg.mode = cmdRequest.m_modeID;
    controlCtrlCmdMsg.cmdId = cmdRequest.m_cmdID;
    controlCtrlCmdMsg.settingParamMap = cmdRequest.m_paramdataMap;
    controlCtrlCmdMsg.multiParamMap = cmdRequest.multiParamMap;

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();

    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = cmdRequest.m_systemNumb;
    targetAddr.deviceNumb = cmdRequest.m_deviceNumb;
    targetAddr.extenNumb = cmdRequest.m_extenNumb;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;

    PackMessage packMsg;
    packMsg.header.msgType = DevMsgType::ProcessControlCmd;
    packMsg.header.sourceAddr = sourceAddr;
    packMsg.header.targetAddr = targetAddr;
    packMsg.header.version = version;
    packMsg.processCtrlCmdMsg = controlCtrlCmdMsg;

    // 组包和推送待发送的命令信息
    packAndPublishCmdInfo(packMsg, pack);
}

void DeviceProcessInfo::acceptClientParamMacroSettingMessage(const ProtocolPack& pack, const ParamMacroRequest& paramRequest)
{
    GroupParamSetMessage paraMacroSetMsg;
    paraMacroSetMsg.modeId = paramRequest.m_modeID;

    for (auto unitID : paramRequest.m_paramMacroInfoMap.keys())
    {
        auto unitParamRequest = paramRequest.m_paramMacroInfoMap[unitID];
        UnitParamSetMessage paraMsg;
        paraMsg.mode = paraMacroSetMsg.modeId;
        paraMsg.unitId = unitID;
        //有效标识所有单元暂时只用一个
        paraMsg.channelValidIdent = paramRequest.m_validIdent;
        paraMsg.settingParamMap = unitParamRequest.m_paramdataMap;

        for (auto targetKey : unitParamRequest.m_multiTargetParamMap.keys())
        {
            auto multiTargetParamMap = unitParamRequest.m_multiTargetParamMap[targetKey];
            paraMsg.multiTargetParamMap[targetKey] = multiTargetParamMap.m_paramdataMap;
        }

        paraMacroSetMsg.unitParamSetMsgMap.insert(unitID, paraMsg);
    }

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();

    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = paramRequest.m_systemNumb;
    targetAddr.deviceNumb = paramRequest.m_deviceNumb;
    targetAddr.extenNumb = paramRequest.m_extenNumb;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;

    PackMessage packMsg;
    packMsg.header.msgType = DevMsgType::GroupParamSetCmd;
    packMsg.header.sourceAddr = sourceAddr;
    packMsg.header.targetAddr = targetAddr;
    packMsg.header.version = version;
    packMsg.groupParamSetMsg = paraMacroSetMsg;

    // 组包和推送待发送的命令信息
    packAndPublishCmdInfo(packMsg, pack);
}

void DeviceProcessInfo::packAndPublishCmdInfo(const PackMessage& packMsg, const ProtocolPack& pack)
{
    const auto& targetAddr = packMsg.header.targetAddr;

    auto extenName = GlobalData::getExtensionName(targetAddr);
    int unitOrCmdId;
    auto cmdTypeDesc = DevProtocolEnumHelper::devMsgTypeToDescStr(packMsg.header.msgType);
    QString unitOrCmdName;
    if (packMsg.header.msgType == DevMsgType::UnitParameterSetCmd)
    {
        unitOrCmdName = GlobalData::getUnitName(targetAddr, packMsg.unitParamSetMsg.mode, packMsg.unitParamSetMsg.unitId);
        unitOrCmdId = packMsg.unitParamSetMsg.unitId;
    }
    else if (packMsg.header.msgType == DevMsgType::ProcessControlCmd)
    {
        unitOrCmdName = GlobalData::getCmdName(targetAddr, packMsg.processCtrlCmdMsg.mode, packMsg.processCtrlCmdMsg.cmdId);
        unitOrCmdId = packMsg.processCtrlCmdMsg.cmdId;
    }

    auto cmdInfo = QString("%1：%2 %3").arg(extenName).arg(unitOrCmdName).arg(cmdTypeDesc);

    QByteArray packData;
    QString erroMsg;
    bool flag = PacketHandler::instance().pack(packMsg, packData, erroMsg);
    if (!flag)
    {
        auto msg = QString("组包出错：%1").arg(erroMsg);
        SystemLogPublish::errorMsg(QString("%1 (%2)").arg(cmdInfo).arg(msg));
        return;
    }
    m_impl->recordingMessageSrc(packMsg.header.targetAddr, unitOrCmdId, pack);
    emit sg_sendByteArrayToDevice(packData, cmdInfo);

    // SystemLogPublish::infoMsg(cmdInfo);
}

#if 1
void DeviceProcessInfo::sendStatusToHealthManager(const UnpackMessage& unpackMsg)
{
    auto sourceAddr = unpackMsg.header.sourceAddr;
    auto modeId = unpackMsg.extenStatusReportMsg.modeId;
    auto modeCtrl = GlobalData::getMode(sourceAddr, modeId);

    DeviceStatusData statusData;
    statusData.deviceID = DeviceID(sourceAddr.systemNumb, sourceAddr.deviceNumb, sourceAddr.extenNumb);
    statusData.deviceName = GlobalData::getExtensionName(sourceAddr);
    statusData.modeId = modeId;

    for (auto unitId : unpackMsg.extenStatusReportMsg.unitReportMsgMap.keys())
    {
        auto unitReportMsg = unpackMsg.extenStatusReportMsg.unitReportMsgMap[unitId];

        auto unitName = GlobalData::getUnitName(sourceAddr, statusData.modeId, unitId);
        UnitStatusParam unitStatusParam;

        auto unit = modeCtrl.unitMap[unitId];
        for (auto attr : unit.queryResultList)
        {
            auto key = QString("%1%2").arg(unitId).arg(attr.pid, 3, 10, QChar('0'));
            ParamStatus paramStatus;
            paramStatus.parName = attr.desc;
            paramStatus.value = unitReportMsg.paramMap[attr.id];
            paramStatus.unit = attr.unit;
            paramStatus.minValue = attr.minValue;
            paramStatus.maxValue = attr.maxValue;

            if (attr.displayFormat == DisplayFormat::DisplayFormat_Enum || attr.displayFormat == DisplayFormat::DisplayFormat_LED)
            {
                for (auto enumEntry : unit.enumMap.value(attr.enumType).emumEntryList)
                {
                    if (enumEntry.uValue == paramStatus.value)
                    {
                        QStringList list = enumEntry.desc.split(":", QString::SkipEmptyParts);  // 分割状态灯 正常:G
                        if (!list.isEmpty())
                        {
                            paramStatus.enumValueDesc = list.at(0);
                            break;
                        }
                    }
                }
            }

            unitStatusParam.paramStatusMap[key] = paramStatus;
        }

        statusData.unitParamMap[unitName] = unitStatusParam;
    }
    QString json;
    json << statusData;

    RedisHelper::getInstance().publish(DeviceStatusDataChannel, json);
}

#else
void DeviceProcessInfo::sendStatusToHealthManager(const UnpackMessage& unpackMsg)
{
    auto sourceAddr = unpackMsg.header.sourceAddr;
    auto modeId = unpackMsg.extenStatusReportMsg.modeId;
    auto modeCtrl = GlobalData::getMode(sourceAddr, modeId);

    DeviceStatusData statusData;
    statusData.deviceID = DeviceID(sourceAddr.systemNumb, sourceAddr.deviceNumb, sourceAddr.extenNumb);
    statusData.deviceName = GlobalData::getExtensionName(sourceAddr);
    statusData.modeId = modeId;

    auto paramFunc =  [](UnitStatusParam& unitStatusParam,const QList<ParameterAttribute>& resultList,
            const QMap<QString, DevEnum>&enumMap ,const QVariantMap&paramData,
            int unitID,int targetID)
    {
        for (auto& attr : resultList)
        {
            QString key;
            if(targetID <= 0)
            {
                key = QString("%1%2%3").arg(unitID).arg(attr.pid, 3, 10, QChar('0'));
            }
            else
            {
                key = QString("%1%2%3").arg(unitID).arg(attr.pid, 3, 10, QChar('0')).arg(targetID);
            }

            ParamStatus paramStatus;
            paramStatus.parName = attr.desc;
            paramStatus.value = paramData.value(attr.id);
            paramStatus.unit = attr.unit;
            paramStatus.minValue = attr.minValue;
            paramStatus.maxValue = attr.maxValue;

            if (attr.displayFormat == DisplayFormat::DisplayFormat_Enum || attr.displayFormat == DisplayFormat::DisplayFormat_LED)
            {
                for (auto& enumEntry : enumMap.value(attr.enumType).emumEntryList)
                {
                    if (enumEntry.uValue == paramStatus.value)
                    {
                        QStringList list = enumEntry.desc.split(":", QString::SkipEmptyParts);  // 分割状态灯 正常:G
                        if (!list.isEmpty())
                        {
                            paramStatus.enumValueDesc = list.at(0);
                            break;
                        }
                    }
                }
            }

            unitStatusParam.paramStatusMap[key] = paramStatus;
        }
    };

    for (auto iter = unpackMsg.extenStatusReportMsg.unitReportMsgMap.begin();
         iter != unpackMsg.extenStatusReportMsg.unitReportMsgMap.end();++iter)
    {
        auto unitId = iter.key();
        auto &unitReportMsg = iter.value();

        auto unitName = GlobalData::getUnitName(sourceAddr, statusData.modeId, unitId);
        UnitStatusParam unitStatusParam;

        auto unit = modeCtrl.unitMap[unitId];
        paramFunc(unitStatusParam, unit.queryResultList, unit.enumMap, unitReportMsg.paramMap, unitId, -1);

        for(auto targetIter = unit.queryRecycleMap.begin();
            targetIter != unit.queryRecycleMap.end();++targetIter)
        {
            auto targetNo = targetIter.key();
            auto&targetData = targetIter.value();
            if(unitReportMsg.multiTargetParamMap.isEmpty())
            {
                continue;
            }
            auto targetParamList = unitReportMsg.multiTargetParamMap.first();
            paramFunc(unitStatusParam, targetData, unit.enumMap, targetParamList, unitId, targetNo);

        }

        statusData.unitParamMap[unitName] = unitStatusParam;
    }


    QString json;
    json << statusData;

    RedisHelper::getInstance().publish(DeviceStatusDataChannel, json);
}
#endif
