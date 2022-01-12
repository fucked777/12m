#ifndef STATIONNETCENTERCMD_H
#define STATIONNETCENTERCMD_H

#include "StationNetCenterType.h"
#include <QThread>

struct StationNetCenterPackInfo
{
    quint64 timeEpoch{ 0 }; /* 记录收到数据的时间戳 */
    QByteArray rawData;
    QByteArray realData;
    PDXPHeader header;
};
// struct StationNetCenterCmdInfo
//{
//    QList<StationNetCenterPackInfo> packList;
//    int count{ 0 }; /* 计数 */
//};

class GlobalParamMacroData;
struct DeviceID;
class StationNetCenterCmdImpl;
class StationNetCenterCmd : public QObject
{
    Q_OBJECT
public:
    explicit StationNetCenterCmd(QObject* parent = nullptr);
    ~StationNetCenterCmd();
    void recvStationNetCenter(const QByteArray& data);

private:
    // 发送远程控制命令应答
    void sendRemoteControlCmdReply(const ZwzxRemoteControlCmdReply& remoteCmdReply);
    void sendRemoteControlCmdReply(const PDXPHeader& header, ZwzxRemoteControlCmdReplyResult result);
    /* 发送远程控制命令执行结果 */
    void sendRemoteControlCmdResult(const ZwzxRemoteControlCmdResult& remoteCmdResult);
    void sendRemoteControlCmdResult(const ZwzxRemoteControlCMD& remoteCmd, ZwzxRemoteControlCmdReplyResult result);

    // 生成顺序号
    int generateSendIndex();

signals:
    void signalSend2StationNetCenter(const QByteArray& data);
    void signalRemoteControlCmd(const ZwzxRemoteControlCMD&);
    void signalSendToDevice(const QByteArray& data);

private:
    void timerEvent(QTimerEvent* event) override;
    void remoteControlCMDRecv();

private:
    StationNetCenterCmdImpl* m_impl;
};

#endif  // STATIONNETCENTERCMD_H
