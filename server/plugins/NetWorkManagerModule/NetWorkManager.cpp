#include "NetWorkManager.h"
#include "CConverter.h"
#include "CommunicationDefine.h"
#include "CommunicationSerialize.h"
#include "CppMicroServicesUtility.h"
#include "CustomPacketMessageSerialize.h"
#include "DevProtocolSerialize.h"
#include "GlobalData.h"
#include "LocalCommunicationAddr.h"
#include "PacketHandler.h"
#include "PlatformInterface.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "StatusQueryCmdPacker.h"
#include "TcpChannelServer.h"
#include "UdpChannelMulticast.h"
#include "UdpChannelP2P.h"
#include "Utility.h"
#include "SubscriberHelper.h"
#include "HeartbeatMessageSerialize.h"
#include "CommunicationDefine.h"
#include "MessagePublish.h"
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QtConcurrent>
#include <atomic>
#include <QApplication>

struct NetSendImplPack
{
    quint32 id;
    ProtocolPack protocolPack;
    explicit NetSendImplPack()
        : id(0)
    {
    }
    explicit NetSendImplPack(const ProtocolPack& pack)
        : id(0)
        , protocolPack(pack)
    {
    }
};

/* 这里信源地址 或者目的地址的类型都可以的 */
using ChannelMapType = QMap<quint32, BaseChannel*>;
class NetWorkManagerImpl
{
    DIS_COPY_MOVE(NetWorkManagerImpl)
public:
    cppmicroservices::BundleContext context;
    NetWorkManager* self;
    /* 当前的网络通道 */
    ChannelMapType channelMap;

    qint32 sendMaxLen{ 1000 }; /* 队列的最大长度,超过此长度后会清空队列 */
    /* 发送队列
     */
    std::atomic_bool sendFlag{ false };
    std::atomic_bool sendEndFlag{ true };
    QMutex sendMutex;
    QWaitCondition sendCondition;
    QList<NetSendImplPack> sendList;

    int timerID{ -1 };
    QList<NetSendImplPack> statusQueryCmdList; /* 定时状态查询队列 */
    MasterSlaveStatus masterStatus{MasterSlaveStatus::Unknown};

    // QString onlineStatus;
    /* 测试数据源 */
    QList<QByteArray> testDataSource;

public:
    inline NetWorkManagerImpl(NetWorkManager* self_)
        : self(self_)
    {
        PacketHandler::instance().appendPacker(new StatusQueryCmdPacker());
    }
    inline ~NetWorkManagerImpl() {}

    /* 创建通讯通道 */
    static BaseChannel* createChannel(const CommunicationInfo& info);
    Optional<ChannelMapType> createCommunicationChannel(const CommunicationInfoMap&);
    void stopAll();
    OptionalNotValue reload();
    void masterSlave(const HeartbeatMessage& msg);

    void startStatusTimer();
    void stopStatusTimer();

    void sendThread();
    /* 保存原始数据到文件 测试使用 */
    void saveRawData(const CommunicationInfo& channelInfo, const QByteArray& data);
    void initTestDataSource();
};
BaseChannel* NetWorkManagerImpl::createChannel(const CommunicationInfo& info)
{
    switch (info.type)
    {
    case CommunicationType::Unknow:
    {
        break;
    }
    case CommunicationType::UDPGB:
    {
        return new UdpChannelMulticast(info, nullptr);
    }
    case CommunicationType::UDPP2P:
    {
        return new UdpChannelP2P(info, nullptr);
    }
    case CommunicationType::TCPServer:
    {
        break;
    } /* TCP服务端 */
    case CommunicationType::TCPClient:
    {
        break;
    } /* TCP客户端 */
    }
    return nullptr;
}

void NetWorkManagerImpl::saveRawData(const CommunicationInfo& channelInfo, const QByteArray& data)
{
    auto tempDir = PlatformConfigTools::configShare("DevRawData");
    QDir dir(tempDir);
    if (!dir.exists())
    {
        if (!dir.mkpath(tempDir))
        {
            return;
        }
    }
    auto filePath = dir.absoluteFilePath(QString::number(channelInfo.id, 16));
    if (QFile::exists(filePath))
    {
        return;
    }
    qWarning() << filePath;

    QFile file(filePath);
    if (!file.open(QFile::ReadWrite))
    {
        return;
    }
    file.write(data);
    file.flush();
}
/* 测试数据源 */
void NetWorkManagerImpl::initTestDataSource()
{
    auto tempDir = PlatformConfigTools::configShare("DevRawData");
    QDir dir(tempDir);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setSorting(QDir::DirsFirst);

    auto dirInfoList = dir.entryInfoList();

    for (auto& fileInfoItem : dirInfoList)
    {
        QFile file(fileInfoItem.absoluteFilePath());
        if (!file.open(QFile::ReadOnly))
        {
            continue;
        }
        testDataSource << file.readAll();
    }
}

Optional<ChannelMapType> NetWorkManagerImpl::createCommunicationChannel(const CommunicationInfoMap& configs)
{
    using ResType = Optional<ChannelMapType>;
    DevProtocolHeader header;
    header.did = LocalCommunicationAddr::devAddrProtocol();

    int index = 0;
    ChannelMapType channelMap;
    for (auto& item : configs.infoMap)
    {
        auto channel = createChannel(item);
        if (channel == nullptr)
        {
            continue;
        }
        auto find = channelMap.find(item.id);
        if (find != channelMap.end())
        {
            delete channel;
            continue;
        }

        channel->setChanelNum(index++); /* 设置当前的编号 */
        channel->setCompareHead(header);
        QObject::connect(channel, &BaseChannel::sg_recvPack, [=](const CommunicationInfo& channelInfo, const QByteArray& data) {
            if (!item.recvDataDesModule.isEmpty())
            {
                // 测试数据捕获
                // saveRawData(channelInfo, data);
                ProtocolPack sendPack;
                sendPack.desID = item.recvDataDesModule;
                sendPack.operation = "AcceptDeviceMessage";
                sendPack.operationACK = QString();
                sendPack.module = true;
                sendPack.data = CommunicationHelper::toByteArray(channelInfo.channelNo, data);
                self->silenceSend(sendPack);
            }
        });
        QObject::connect(channel, &BaseChannel::sg_recvErrorPack, [=](const QByteArray& data, const QString& msg) {
            if (!item.recvDataDesModule.isEmpty())
            {
                ProtocolPack sendPack;
                sendPack.desID = item.recvDataDesModule;
                sendPack.operation = "AcceptErrorDeviceMessage";
                sendPack.operationACK = QString();
                sendPack.module = true;
                sendPack.data = data;
                self->silenceSend(sendPack);

                qWarning() << "错误类型:" << msg;
            }
        });

        QObject::connect(channel, &BaseChannel::sg_channelRecvDataChange, self, &NetWorkManager::sg_channelRecvDataChange);
        QObject::connect(channel, &BaseChannel::sg_channelSendDataChange, self, &NetWorkManager::sg_channelSendDataChange);
        channelMap.insert(item.id, channel);
        channel->start();
    }
    return ResType(channelMap);
}
void NetWorkManagerImpl::stopAll()
{
    stopStatusTimer();
    sendFlag = false;
    sendCondition.wakeAll();
    /* 等待线程退出 */
    while (!sendEndFlag)
    {
        QThread::msleep(10);
        QApplication::processEvents();
    }

    for (auto& channel : channelMap)
    {
        channel->stop();
        delete channel;
    }

    ChannelMapType().swap(channelMap);
}
void NetWorkManagerImpl::startStatusTimer()
{
    if (timerID == -1)
    {
        timerID = self->startTimer(1000);
    }
}
void NetWorkManagerImpl::stopStatusTimer()
{
    if (timerID != -1)
    {
        self->killTimer(timerID);
        timerID = -1;
    }
}
void NetWorkManagerImpl::masterSlave(const HeartbeatMessage& msg)
{
    if(masterStatus == msg.status)
    {
        return;
    }
    masterStatus = msg.status;
    /* 主机先停止再启动
     * 备机直接停止
    */
    if(msg.status == MasterSlaveStatus::Master)
    {
        SystemLogPublish::infoMsg(QString("主备切换:%1主机设备网络重置中").arg(msg.currentMasterIP));
        /* 特殊的重载 */
        /* 首先停止并删除所有的网络通道 */
        stopAll();
        startStatusTimer();
        /* 重新加载通道数据信息 */
        auto infoMap = GlobalData::communicationInfo();

        /* 替换本地IP */
        if(msg.currentMasterIP.isEmpty())
        {
            for(auto&item : infoMap.infoMap)
            {
                item.localIP = msg.currentMasterIP;
            }
        }
        auto res = NetWorkManagerImpl::createCommunicationChannel(infoMap);
        if(!res)
        {
            SystemLogPublish::infoMsg(QString("主备切换:%1主机设备网络重置错误%2").arg(msg.currentMasterIP, res.msg()));
            return;
        }
        channelMap = res.value();
        sendFlag = true;

        /* 发送线程启动 */
        QtConcurrent::run(this, &NetWorkManagerImpl::sendThread);
        SystemLogPublish::infoMsg(QString("主备切换:%1主机设备网络重置完成").arg(msg.currentMasterIP));
    }
    else
    {
        SystemLogPublish::infoMsg("主备切换:设备备机网络停止中");
        stopAll();
        SystemLogPublish::infoMsg("主备切换:设备备机网络停止完成");
    }
}

OptionalNotValue NetWorkManagerImpl::reload()
{
    /* 首先停止并删除所有的网络通道 */
    stopAll();
    /* 重新加载通道数据信息 */
    auto infoMap = GlobalData::communicationInfo();

    auto res = NetWorkManagerImpl::createCommunicationChannel(infoMap);
    if(!res)
    {
        return OptionalNotValue(res.errorCode(), res.msg());
    }
    channelMap = res.value();

    /* 发送线程启动 */
    QtConcurrent::run(this, &NetWorkManagerImpl::sendThread);
    return OptionalNotValue();
}
void NetWorkManagerImpl::sendThread()
{
    /* 退出后flag自动归位 */
    AtomicFlagHelper endFlag(sendEndFlag, false);
    AtomicFlagHelper runFlag(sendFlag, true);

    while (true)
    {
        sendMutex.lock();
        if (sendList.isEmpty())
        {
            sendCondition.wait(&sendMutex);
        }
        /* 退出标志 */
        if (!sendFlag)
        {
            sendMutex.unlock();
            return;
        }
        auto implPack = sendList.first();
        sendList.removeFirst();
        sendMutex.unlock();

        auto find = channelMap.find(implPack.id);
        if (find == channelMap.end())
        {
            implPack.protocolPack.data.clear();
            auto errMsg = QString("%1:%2").arg("无目标设备").arg(implPack.id, 0, 16);
            OptionalNotValue ackMsg(ErrorCode::NotFound, errMsg);
            implPack.protocolPack.data << ackMsg;
            self->silenceSendACK(implPack.protocolPack);
            continue;
        }

        auto channel = find.value();
        auto sendRes = channel->sendData(implPack.protocolPack.data);

        implPack.protocolPack.data.clear();
        implPack.protocolPack.data << sendRes;
        self->silenceSendACK(implPack.protocolPack);
    }
}
/**********************************************************************************************************/
/**********************************************************************************************************/
/**********************************************************************************************************/
/**********************************************************************************************************/
/**********************************************************************************************************/
/**********************************************************************************************************/
NetWorkManager::NetWorkManager(cppmicroservices::BundleContext context)
    : INetMsg("NetWorkManagerThread", nullptr)
    , m_impl(new NetWorkManagerImpl(this))
{
    qRegisterMetaType<NetStatusItemInfo>("const NetStatusItemInfo&");
    qRegisterMetaType<NetStatusItemInfo>("NetStatusItemInfo&");
    qRegisterMetaType<NetStatusItemInfo>("NetStatusItemInfo");

    registerSubscribe("sendDataToDevice", &NetWorkManager::sendData, this);
    registerSubscribe("sendDataToOtherDevice", &NetWorkManager::sendOtherData, this);
    registerSubscribe("reload", &NetWorkManager::reload, this);

    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_masterSlave,this,&NetWorkManager::masterSlave);
    m_impl->context = context;

    m_impl->reload();
    /* 初始化状态发送列表 */
    initStatusQueryCmdPacker();
    // startStatusTimer();
    /* 初始化测试数据源 */
    // m_impl->initTestDataSource();
}
NetWorkManager::~NetWorkManager()
{
    stopStatusTimer();
    stopAll();
    delete m_impl;
    m_impl = nullptr;
}
void NetWorkManager::stopAll() { m_impl->stopAll(); }

void NetWorkManager::sendData(const ProtocolPack& pack)
{
    m_impl->sendMutex.lock();
    /* 堵塞了 */
    if (m_impl->sendList.size() > m_impl->sendMaxLen)
    {
        SystemLogPublish::warningMsg(QString("网络数据堆积数据量超过:%1").arg(m_impl->sendMaxLen));
        m_impl->sendList.clear();
    }

    /* 20210804 wp?? 套了一层做中转 */
    NetSendImplPack protocal(pack);
    protocal.id = ProtocolHeaderBit::didDevID(pack.data);
    m_impl->sendList.append(protocal);
    m_impl->sendCondition.wakeAll();

    m_impl->sendMutex.unlock();
}

void NetWorkManager::sendOtherData(const ProtocolPack& pack)
{
    /* 向设备发送数据 20210804 wp?
     * data 发送给别的设备的数据 协议未知 收到的数据固定为 ID(固定为2字节)+协议数据
     * ack OptionalNotValue
     */
    m_impl->sendMutex.lock();
    /* 堵塞了 */
    if (m_impl->sendList.size() > m_impl->sendMaxLen)
    {
        m_impl->sendList.clear();
    }
    NetSendImplPack protocal(pack);
    protocal.protocolPack.data = pack.data.mid(2);
    /* 这里使用的时候需要注意一下大小端问题 */
    protocal.id = CConverter::fromByteArray<quint16>(pack.data.mid(0, 2));

    m_impl->sendList.append(protocal);
    m_impl->sendCondition.wakeAll();

    m_impl->sendMutex.unlock();
}

/* 重新加载,网络配置 */
void NetWorkManager::reload(const ProtocolPack& pack)
{
    auto res = m_impl->reload();
    auto ack = pack;
    ack.data << res;
    silenceSendACK(ack);
}
void NetWorkManager::setSendMaxLen(int max)
{
    if (m_impl->sendFlag)
    {
        return;
    }
    if (max <= 0)
    {
        return;
    }
    m_impl->sendMaxLen = max;
}

void NetWorkManager::initStatusQueryCmdPacker()
{
    auto systemMap = GlobalData::getSystemMap();
    for (auto sysIter = systemMap.begin(); sysIter != systemMap.end(); ++sysIter)
    {
        auto& sysID = sysIter.key();
        auto& system = sysIter.value();
        for (auto devIter = system.deviceMap.begin(); devIter != system.deviceMap.end(); ++devIter)
        {
            auto& devID = devIter.key();
            auto& device = devIter.value();
            for (auto extendIter = device.extensionMap.begin(); extendIter != device.extensionMap.end(); ++extendIter)
            {
                PackMessage packMsg;
                auto& extendID = extendIter.key();

                packMsg.header.msgType = DevMsgType::StatusQueryCmd;
                packMsg.header.sourceAddr = LocalCommunicationAddr::devAddrMessage();

                packMsg.header.targetAddr = LocalCommunicationAddr::devAddrMessage();
                packMsg.header.targetAddr.systemNumb = sysID;
                packMsg.header.targetAddr.deviceNumb = devID;
                packMsg.header.targetAddr.extenNumb = extendID;

                /* 20210804 wp?? 处理发送协议加了一层 */
                NetSendImplPack protocal;
                QString erroMsg;
                PacketHandler::instance().pack(packMsg, protocal.protocolPack.data, erroMsg);

                protocal.id = ProtocolHeaderBit::didDevID(protocal.protocolPack.data);
                // protocal.srcID = "";
                // protocal.desID = "NetWorkManager";
                // protocal.module = true;
                // protocal.operation = "sendDataToDevice";
                m_impl->statusQueryCmdList.append(protocal);
            }
        }
    }
    qWarning() << "*****当前状态查询条数:" << m_impl->statusQueryCmdList.size();
}
void NetWorkManager::startStatusTimer()
{
    m_impl->startStatusTimer();
}
void NetWorkManager::stopStatusTimer()
{
    m_impl->stopStatusTimer();
}
void NetWorkManager::devStatusQuery()
{
    /* 设备状态查询 */
    m_impl->sendMutex.lock();
    /* 堵塞了 */
    if (m_impl->sendList.size() > m_impl->sendMaxLen)
    {
        m_impl->sendList.clear();
    }
    m_impl->sendList.append(m_impl->statusQueryCmdList);
    m_impl->sendCondition.wakeAll();
    // RedisHelper::getInstance().getData("CenterDeviceOnline", m_impl->onlineStatus);

    m_impl->sendMutex.unlock();
}
void NetWorkManager::testDataSourcePush()
{
    for (auto& item : m_impl->testDataSource)
    {
        /* 获取设备ID */
        auto srcID = ProtocolHeaderBit::sidDevID(item);
        auto channel = m_impl->channelMap.value(srcID);
        if (channel != nullptr)
        {
            auto channelInfo = channel->channelInfo();
            ProtocolPack sendPack;
            sendPack.desID = channelInfo.recvDataDesModule;
            sendPack.operation = "AcceptDeviceMessage";
            sendPack.operationACK = QString();
            sendPack.module = true;
            sendPack.data = CommunicationHelper::toByteArray(channelInfo.channelNo, item);
            silenceSend(sendPack);
        }
    }
}

void NetWorkManager::timerEvent(QTimerEvent* /*event*/)
{
    /* 测试数据推送 */
    // testDataSourcePush();
    /* 设备状态查询 */
    devStatusQuery();
    /* 定时推送服务器在线状态 */
    GlobalData::setServiceOnlineStatus();
}
void NetWorkManager::masterSlave(const HeartbeatMessage& msg)
{
    m_impl->masterSlave(msg);
}

