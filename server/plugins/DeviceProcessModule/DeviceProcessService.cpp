#include "DeviceProcessService.h"

#include "CConverter.h"
#include "CommunicationSerialize.h"
#include "ControlCmdResponseUnpacker.h"
#include "ControlResultReportUnpacker.h"
#include "CppMicroServicesUtility.h"
#include "CustomPacketMessageSerialize.h"
#include "DevProtocol.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessInfo.h"
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
#include <QThread>
#include <QtConcurrent>

class DeviceProcessServiceImpl
{
public:
    struct DeviceProcessThreadInfo
    {
        QThread* thread{ nullptr };
        DeviceProcessInfo* deviceProcessInfo{ nullptr };
    };

    /*
     * key : 通道号
     * value 对应的通道线程
     */
    qint32 channelNum{ 0 };
    QVector<DeviceProcessThreadInfo> deviceProcessInfoList;
    QMap<quint32, qint32> deviceIDMapChannel;
    cppmicroservices::BundleContext context;
};
DeviceProcessService::DeviceProcessService(const QString& threadName, cppmicroservices::BundleContext context)
    : INetMsg(threadName, nullptr)
    , m_impl(new DeviceProcessServiceImpl)
{
    m_impl->context = context;

    qRegisterMetaType<UnitParamRequest>("UnitParamRequest");
    qRegisterMetaType<UnitParamRequest>("UnitParamRequest&");
    qRegisterMetaType<UnitParamRequest>("const UnitParamRequest&");

    qRegisterMetaType<CmdRequest>("CmdRequest");
    qRegisterMetaType<CmdRequest>("CmdRequest&");
    qRegisterMetaType<CmdRequest>("const CmdRequest&");

    qRegisterMetaType<ParamMacroRequest>("ParamMacroRequest");
    qRegisterMetaType<ParamMacroRequest>("ParamMacroRequest&");
    qRegisterMetaType<ParamMacroRequest>("const ParamMacroRequest&");

    /* 注册解包打包器 */
    PacketHandler::instance().appendUnpacker(new ExtensionStatusReportUnpacker());
    PacketHandler::instance().appendUnpacker(new ControlCmdResponseUnpacker());
    PacketHandler::instance().appendUnpacker(new ControlResultReportUnpacker());

    PacketHandler::instance().appendPacker(new ProcessControlCmdPacker());
    PacketHandler::instance().appendPacker(new UnitParamSetPacker());
    PacketHandler::instance().appendPacker(new StatusQueryCmdPacker());
    PacketHandler::instance().appendPacker(new GroupParamSetPacker());
    /* 启动解析线程 */
    /* 获取通讯配置 */
    auto communicationInfo = GlobalData::communicationInfo();
    auto channelNum = communicationInfo.channelNum <= 0 ? 1 : communicationInfo.channelNum;
    m_impl->deviceProcessInfoList.resize(channelNum);
    m_impl->channelNum = channelNum;
    for (auto i = 0; i < channelNum; ++i)
    {
        auto thread = new QThread;
        auto deviceProcessInfo = new DeviceProcessInfo;
        connect(deviceProcessInfo, &DeviceProcessInfo::sg_cmdACK, this, &DeviceProcessService::silenceSendACK);
        connect(deviceProcessInfo, &DeviceProcessInfo::sg_sendByteArrayToDevice, this, &DeviceProcessService::sendByteArrayToDevice);
        deviceProcessInfo->moveToThread(thread);
        thread->start();

        DeviceProcessServiceImpl::DeviceProcessThreadInfo processInfo;
        processInfo.thread = thread;
        processInfo.deviceProcessInfo = deviceProcessInfo;
        m_impl->deviceProcessInfoList[i] = processInfo;
    }
    /* 转换一下通道号和设备ID */
    for (auto& item : communicationInfo.infoMap)
    {
        m_impl->deviceIDMapChannel[item.id] = item.channelNo;
    }

    /* 注册信号回调 */
    registerSubscribe("AcceptDeviceMessage", &DeviceProcessService::acceptDeviceMessage, this);
    registerSubscribe("AcceptErrorDeviceMessage", &DeviceProcessService::acceptErrorDeviceMessage, this);
    registerSubscribe("AcceptClientUnitSettingMessage", &DeviceProcessService::acceptClientUnitSettingMessage, this);
    registerSubscribe("AcceptClientCmdSettingMessage", &DeviceProcessService::acceptClientCmdSettingMessage, this);
    registerSubscribe("AcceptClientParamMacroSettingMessage", &DeviceProcessService::acceptClientParamMacroSettingMessage, this);
}

DeviceProcessService::~DeviceProcessService()
{
    for (auto& item : m_impl->deviceProcessInfoList)
    {
        item.thread->quit();
        item.thread->wait();
    }
    for (auto& item : m_impl->deviceProcessInfoList)
    {
        delete item.deviceProcessInfo;
        delete item.thread;
    }
    m_impl->deviceProcessInfoList.clear();
}

void DeviceProcessService::sendByteArrayToDevice(const QByteArray& array, const QString& cmdInfo)
{
    /************************** 发送数据给设备 **************************/
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = array;
    auto sendRes = send(protocal);
    if (!sendRes)
    {
        auto msg = QString("%1 (命令发送错误：%2)").arg(cmdInfo, sendRes.msg());
        SystemLogPublish::errorMsg(msg);
    }
}

void DeviceProcessService::acceptDeviceMessage(const ProtocolPack& bak)
{
    auto pack = bak;
    qint32 channelNo = 1;
    CommunicationHelper::fromByteArray(channelNo, pack.data);
    if (channelNo <= 0 || channelNo > m_impl->channelNum)
    {
        auto msg = QString("通讯配置文件错误 未找到通道号:%1").arg(channelNo);
        SystemLogPublish::errorMsg(msg);
        return;
    }
    --channelNo;
    auto info = m_impl->deviceProcessInfoList.at(channelNo);
    emit info.deviceProcessInfo->sg_acceptDeviceMessage(pack);
}

void DeviceProcessService::acceptErrorDeviceMessage(const ProtocolPack& pack)
{
    qWarning() << pack.srcID << "    "
               << "设备原始数据异常:" << pack.data.toHex();
}

void DeviceProcessService::acceptClientUnitSettingMessage(const ProtocolPack& bak)
{
    auto pack = bak;
    UnitParamRequest paramRequest;
    pack.data >> paramRequest;

    auto deviceIDNum = DeviceID(paramRequest.m_systemNumb, paramRequest.m_deviceNumb, paramRequest.m_extenNumb).toUInt();
    auto channelNo = m_impl->deviceIDMapChannel.value(deviceIDNum);
    if (channelNo <= 0 || channelNo > m_impl->channelNum)
    {
        auto msg = QString("通讯配置文件错误 设备ID:0x%1 未找到通道号:%2").arg(deviceIDNum, 0, 16).arg(channelNo);
        SystemLogPublish::errorMsg(msg);
        return;
    }
    --channelNo;
    auto info = m_impl->deviceProcessInfoList.at(channelNo);
    emit info.deviceProcessInfo->sg_acceptClientUnitSettingMessage(pack, paramRequest);
}

void DeviceProcessService::acceptClientCmdSettingMessage(const ProtocolPack& bak)
{
    auto pack = bak;
    CmdRequest cmdRequest;
    pack.data >> cmdRequest;

    auto deviceIDNum = DeviceID(cmdRequest.m_systemNumb, cmdRequest.m_deviceNumb, cmdRequest.m_extenNumb).toUInt();
    auto channelNo = m_impl->deviceIDMapChannel.value(deviceIDNum);
    if (channelNo <= 0 || channelNo > m_impl->channelNum)
    {
        auto msg = QString("通讯配置文件错误 设备ID:0x%1 未找到通道号:%2").arg(deviceIDNum, 0, 16).arg(channelNo);
        SystemLogPublish::errorMsg(msg);
        return;
    }
    --channelNo;
    auto info = m_impl->deviceProcessInfoList.at(channelNo);
    emit info.deviceProcessInfo->sg_acceptClientCmdSettingMessage(pack, cmdRequest);
}

void DeviceProcessService::acceptClientParamMacroSettingMessage(const ProtocolPack& bak)
{
    auto pack = bak;
    ParamMacroRequest paramRequest;
    pack.data >> paramRequest;

    auto deviceIDNum = DeviceID(paramRequest.m_systemNumb, paramRequest.m_deviceNumb, paramRequest.m_extenNumb).toUInt();
    auto channelNo = m_impl->deviceIDMapChannel.value(deviceIDNum);
    if (channelNo <= 0 || channelNo > m_impl->channelNum)
    {
        auto msg = QString("通讯配置文件错误 设备ID:0x%1 未找到通道号:%2").arg(deviceIDNum, 0, 16).arg(channelNo);
        SystemLogPublish::errorMsg(msg);
        return;
    }
    --channelNo;
    auto info = m_impl->deviceProcessInfoList.at(channelNo);
    emit info.deviceProcessInfo->sg_acceptClientParamMacroSettingMessage(pack, paramRequest);
}
