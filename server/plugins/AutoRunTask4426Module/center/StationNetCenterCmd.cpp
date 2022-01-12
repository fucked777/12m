#include "StationNetCenterCmd.h"
#include "CConverter.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PDXPMessageDefine.h"
#include "PackPDXPHeader.h"
#include "RedisHelper.h"
#include "StationNetCenterRemoteCmd.h"
#include "StationNetCenterXmlReader.h"
#include "StationResManagementDefine.h"
#include "GlobalData.h"

#include <QApplication>

/* 时间的误差范围 */
static constexpr auto timeRangeMS = 800;
class StationNetCenterCmdImpl
{
public:
    /* 远控命令 */
    QThread* remoteCmdThread{ nullptr };
    StationNetCenterRemoteCmd* remoteCmdExecFunc{ nullptr };
    SNCCmdMap sncCmdMap;                              /* 配置命令的Map */
    QList<StationNetCenterPackInfo> remoteControlCMD; /* 远程控制命令 */
    int timerID{ -1 };

    /* 获取当前的站ID */
    static quint32 getStationID()
    {
//        StationResManagementMap stationResMap;
//        if (!GlobalData::getStationResManagementInfo(stationResMap))
//        {
//            return 0;
//        }
//        if (stationResMap.isEmpty())
//        {
//            return 0;
//        }
//        auto info = stationResMap.first();
        return 0x66043200;//info.ttcUACAddr.toUInt(nullptr, 16);
    }
    // 相对于2000年1月1日的积日，2000年1月1日计为第一天
    static uint getAccumDate()
    {
        QDate startDate(2000, 1, 1);
        return startDate.daysTo(GlobalData::currentDate()) + 1;
    }
};

StationNetCenterCmd::StationNetCenterCmd(QObject* parent)
    : QObject(parent)
    , m_impl(new StationNetCenterCmdImpl)
{
    qRegisterMetaType<ZwzxRemoteControlCMD>("ZwzxRemoteControlCMD");
    qRegisterMetaType<ZwzxRemoteControlCMD>("const ZwzxRemoteControlCMD&");
    qRegisterMetaType<ZwzxRemoteControlCMD>("ZwzxRemoteControlCMD&");

    StationNetCenterXmlReader xmlReader;
    auto result = xmlReader.getLinkConfig();
    if (!result)
    {
        SystemLogPublish::errorMsg(result.msg(), MACRSTR(MODULE_NAME));
        qWarning() << result.msg();
    }
    else
    {
        m_impl->sncCmdMap = result.value();
    }

    m_impl->remoteCmdThread = new QThread;
    m_impl->remoteCmdExecFunc = new StationNetCenterRemoteCmd(m_impl->sncCmdMap);
    m_impl->remoteCmdExecFunc->moveToThread(m_impl->remoteCmdThread);
    connect(m_impl->remoteCmdExecFunc, &StationNetCenterRemoteCmd::signalSendToDevice, this, &StationNetCenterCmd::signalSendToDevice);
    connect(this, &StationNetCenterCmd::signalRemoteControlCmd, m_impl->remoteCmdExecFunc, &StationNetCenterRemoteCmd::execCmd);
    connect(m_impl->remoteCmdExecFunc, &StationNetCenterRemoteCmd::signalRemoteControlCmdResult, this,
            static_cast<void (StationNetCenterCmd::*)(const ZwzxRemoteControlCMD&, ZwzxRemoteControlCmdReplyResult)>(
                &StationNetCenterCmd::sendRemoteControlCmdResult));

    m_impl->remoteCmdThread->start();
    m_impl->timerID = startTimer(1000);
}
StationNetCenterCmd::~StationNetCenterCmd()
{
    Utility::killTimer(this, m_impl->timerID);
    m_impl->remoteCmdThread->quit();
    m_impl->remoteCmdThread->wait();
    delete m_impl->remoteCmdExecFunc;
    delete m_impl->remoteCmdThread;
    delete m_impl;
}

void StationNetCenterCmd::sendRemoteControlCmdReply(const PDXPHeader& header, ZwzxRemoteControlCmdReplyResult result)
{
    PDXPHeader sendHeader;
    sendHeader.version = 0x1;
    sendHeader.mid = header.mid;
    sendHeader.sid = header.did;
    sendHeader.did = header.sid;
    sendHeader.bid = ZWZX_YCKZMLYD_TYPE;
    sendHeader.no = 0;
    sendHeader.flag = 0;
    sendHeader.reserve = 0;
    sendHeader.date = StationNetCenterCmdImpl::getAccumDate();
    sendHeader.time = GlobalData::currentTime().msecsSinceStartOfDay() * 10;
    sendHeader.len = sizeof(ZwzxRemoteControlCmdReply::infoType) + sizeof(ZwzxRemoteControlCmdReply::result);

    ZwzxRemoteControlCmdReply reply;
    reply.header = sendHeader;
    reply.infoType = sendHeader.bid;
    reply.result = static_cast<quint8>(result);

    sendRemoteControlCmdReply(reply);
}
void StationNetCenterCmd::sendRemoteControlCmdReply(const ZwzxRemoteControlCmdReply& remoteCmdReply)
{
    QByteArray data;
    auto headerData = PackPDXPHeader::packHead(remoteCmdReply.header);
    data.append(headerData);
    data.append(CConverter::toByteArray<quint16>(remoteCmdReply.header.len));

    data.append(CConverter::toByteArray<quint32>(remoteCmdReply.infoType));
    data.append(CConverter::toByteArray<quint8>(remoteCmdReply.result));

    emit signalSend2StationNetCenter(data);
}
void StationNetCenterCmd::sendRemoteControlCmdResult(const ZwzxRemoteControlCmdResult& remoteCmdResult)
{
    QByteArray data;

    auto headerData = PackPDXPHeader::packHead(remoteCmdResult.header);
    data.append(headerData);
    data.append(CConverter::toByteArray<quint16>(remoteCmdResult.header.len));

    data.append(CConverter::toByteArray<quint32>(remoteCmdResult.uac));
    data.append(CConverter::toByteArray<quint32>(remoteCmdResult.t));
    data.append(CConverter::toByteArray<quint8>(remoteCmdResult.pa));
    data.append(CConverter::toByteArray<quint8>(remoteCmdResult.jg));
    data.append(CConverter::toByteArray<quint8>(remoteCmdResult.sjjg));

    emit signalSend2StationNetCenter(data);
}
void StationNetCenterCmd::sendRemoteControlCmdResult(const ZwzxRemoteControlCMD& remoteCmd, ZwzxRemoteControlCmdReplyResult result)
{
    PDXPHeader sendHeader;
    sendHeader.version = 0x1;
    sendHeader.mid = remoteCmd.header.mid;
    sendHeader.sid = remoteCmd.header.did;
    sendHeader.did = remoteCmd.header.sid;
    sendHeader.bid = ZWZX_YCKZMLZXJG_TYPE;
    sendHeader.no = 0;
    sendHeader.flag = 0;
    sendHeader.reserve = 0;
    sendHeader.date = StationNetCenterCmdImpl::getAccumDate();
    sendHeader.time = GlobalData::currentTime().msecsSinceStartOfDay() * 10;

    sendHeader.len = sizeof(ZwzxRemoteControlCmdResult::uac) + /**/
                     sizeof(ZwzxRemoteControlCmdResult::t) +   /**/
                     sizeof(ZwzxRemoteControlCmdResult::pa) +  /**/
                     sizeof(ZwzxRemoteControlCmdResult::jg) +  /**/
                     sizeof(ZwzxRemoteControlCmdResult::sjjg);

    ZwzxRemoteControlCmdResult cmdReplyResult;
    cmdReplyResult.header = sendHeader;
    cmdReplyResult.uac = remoteCmd.header.mid;
    cmdReplyResult.t = (result == ZwzxRemoteControlCmdReplyResult::Refuse ? 0xFFFFFFFF : sendHeader.time);
    cmdReplyResult.pa = remoteCmd.pa;
    cmdReplyResult.jg = remoteCmd.jg;
    cmdReplyResult.sjjg = static_cast<quint8>(result);

    sendRemoteControlCmdResult(cmdReplyResult);
}
int StationNetCenterCmd::generateSendIndex()
{
    static uint index = 0;
    if (index > USHRT_MAX)
    {
        index = 0;
    }
    return ++index;
}
void StationNetCenterCmd::timerEvent(QTimerEvent* /*event*/)
{
    /* 这里的逻辑是这样的,每秒触发一次函数,进来检查,因为收到的数据包是附带了时间戳的,
     * 当三帧的时间戳间隔在timeRangeMS只内,认为其是同一个数据包
     *
     * 当记录时间与当前时间超过1000毫秒认为其不会再有数据包了,直接判
     */
    remoteControlCMDRecv();
}
void StationNetCenterCmd::recvStationNetCenter(const QByteArray& data)
{
    /* 获取当前的站ID */
    auto staticID = StationNetCenterCmdImpl::getStationID();
    if (staticID == 0)
    {
        /* 没有获取到当前的站ID */
        SystemLogPublish::errorMsg("中心指令接收错误:未能获取到当前的站标识");
        return;
    }
    /* 解包 */
    auto headerResult = PackPDXPHeader::unpackHead(data);
    if (!headerResult)
    {
        /* 解包失败,丢弃当前数据 */
        VoiceAlarmPublish::publish("中心命令解析错误");
        SystemLogPublish::errorMsg(QString("中心命令解析错误:%1").arg(headerResult.msg()));
        return;
    }
    auto header = headerResult.value();
    /* wp?? */
    if (header.did != staticID)
    {
        auto msg = QString("中心命令目的地址与当前不符 当前ID：%1 命令ID：%2").arg(staticID, 0, 16).arg(header.did, 0, 16);
        SystemLogPublish::warningMsg(msg);
        return;
    }
    StationNetCenterPackInfo info;
    info.rawData = data;
    info.header = header;
    info.realData = data.mid(32);
    info.timeEpoch = GlobalData::currentDateTime().currentMSecsSinceEpoch();

    auto type = PDXPMESSAGETYPE(header.bid);
    switch (type)
    {
    case ZWZX_YCKZML_TYPE:  // 站网中心远程控制命令
    {
        m_impl->remoteControlCMD << info;
        return;
    }
    case ZWZX_YCKZMLYD_TYPE:  // 站网中心远程控制命令应答
    {
        return;
    }
    case ZWZX_YCKZMLZXJG_TYPE:  // 站网中心远程控制命令执行结果
    {
        return;
    }
    case ZWZX_RWCSSZ_TYPE:  // 站网中心参数设置命令帧
    {
        return;
    }
    case ZWZX_RWCSZXJG_TYPE:  // 任务参数执行结果
    {
        return;
    }
    case ZWZX_SJSHCFML_TYPE:  // 数据事后重发命令
    {
        return;
    }
    case ZWZX_LLJSZ_TYPE:  // 链路监视帧
    {
        /* 链路监视是一秒一发,收到后直接回复 */
        sendRemoteControlCmdReply(header, ZwzxRemoteControlCmdReplyResult::Correct);
        return;
    }
    default: break;
    }

    /* 类型错误,丢弃 */
    VoiceAlarmPublish::publish("中心命令解析错误");
    SystemLogPublish::errorMsg(QString("中心命令解析错误:%1").arg(QString(data.toHex())));
}

void StationNetCenterCmd::remoteControlCMDRecv()
{
    if (m_impl->remoteControlCMD.isEmpty())
    {
        return;
    }

    /* 不为空,如果数量大于等于3,则开始判断数据 */
    QMap<QByteArray, std::tuple<int, StationNetCenterPackInfo>> mTempMap;
    if (m_impl->remoteControlCMD.size() >= 3)
    {
        for (auto i = 0; i < 3; i++)
        {
            auto& data1 = m_impl->remoteControlCMD.at(i);
            auto& data = mTempMap[data1.realData];
            std::get<0>(data) += 1;
            std::get<1>(data) = data1;
        }
        /* 移除前3帧 */
        m_impl->remoteControlCMD.removeFirst();
        m_impl->remoteControlCMD.removeFirst();
        m_impl->remoteControlCMD.removeFirst();
    }
    else
    {
        /* 当前时间戳 */
        auto curMS = GlobalData::currentDateTime().currentMSecsSinceEpoch();
        /* 如果小于3条数据
         * 那就判最后一条数据时间是否与当前时间误差超过1秒
         */
        auto& last = m_impl->remoteControlCMD.last();

        /* 没有超过timeRangeMS,等等 */
        if (curMS - last.timeEpoch < timeRangeMS)
        {
            return;
        }
        /* 超过timeRangeMS了,直接判数据 */
        for (auto& item : m_impl->remoteControlCMD)
        {
            auto& data = mTempMap[item.realData];
            std::get<0>(data) += 1;
            std::get<1>(data) = item;
        }
        m_impl->remoteControlCMD.clear();
    }

    StationNetCenterPackInfo info;
    for (auto iter = mTempMap.begin(); iter != mTempMap.end(); ++iter)
    {
        auto& value = iter.value();
        /* 这里只要命令有两条以上是一样的,就认为是正确的命令 */
        if (std::get<0>(value) > 1)
        {
            info = std::get<1>(value);
            break;
        }
    }
    if (info.rawData.isEmpty())
    {
        /* 帧错误判断失败 */
        VoiceAlarmPublish::publish("中心命令解析错误");
        /* 这里记录错误的帧 */
        int i = 1;
        for (auto& item : mTempMap)
        {
            auto errMsg = QString(std::get<1>(item).rawData.toHex());
            errMsg = QString("中心命令错误帧%1:%2").arg(i).arg(errMsg);
            SystemLogPublish::errorMsg(errMsg);
            ++i;
        }

        return;
    }

    auto msg = QString(info.rawData.toHex());
    msg = QString("收到中心指令:%1").arg(msg);
    SystemLogPublish::specificTipsMsg(msg);

    sendRemoteControlCmdReply(info.header, ZwzxRemoteControlCmdReplyResult::Correct);
    /* 成功了,解析远控指令数据 */
    ZwzxRemoteControlCMD remoteCmd;
    remoteCmd.header = info.header;

    int index = 0;
    remoteCmd.v = CConverter::fromByteArray<quint8>(info.realData.mid(index, 1));
    index += 1;
    remoteCmd.st = CConverter::fromByteArray<quint8>(info.realData.mid(index, 1));
    index += 1;
    remoteCmd.pa = CConverter::fromByteArray<quint8>(info.realData.mid(index, 1));
    index += 1;
    remoteCmd.jg = CConverter::fromByteArray<quint8>(info.realData.mid(index, 1));
    index += 1;
    remoteCmd.res = CConverter::fromByteArray<quint32>(info.realData.mid(index, 4));

    /* 执行命令 */
    emit signalRemoteControlCmd(remoteCmd);
}
