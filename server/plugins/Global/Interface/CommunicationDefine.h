#ifndef COMMUNICATIONINFO_H
#define COMMUNICATIONINFO_H
#include <DevProtocol.h>
#include <QString>

/* 通讯类型 */
enum class CommunicationType : quint32
{
    Unknow = 0,    /* 未知的类型 */
    UDPGB = 1,     /* UDP组播 Group Broadcast */
    UDPP2P = 2,    /* UDP点对点 */
    TCPServer = 3, /* TCP服务端 */
    TCPClient = 4, /* TCP客户端 */
};

/* 通讯 */
struct CommunicationInfo
{
    quint32 id{ 0 }; /* ID */
    CommunicationType type{ CommunicationType::Unknow };
    quint16 recvPort{ 0 };           /* 接收端口 */
    quint16 sendPort{ 0 };           /* 发送端口 */
    QString localIP;                 /* 本地IP */
    QString desc;                    /* 描述 */
    QString desIP;                   /* 目标IP地址 */
    bool isCenterDevice = false;     /* 是否是中心体设备*/
    bool onlyRecvTargetData{ true }; /* 目标地址不是本机接不接收,此选项是防止某些设备目标地址有问题 */
    /* 是否做协议的长度的基本检查,只对设备的通讯有效
     * 比如是高轨的命令你打开检查肯定就是检查不过一直报错
     */
    bool devProtocolCheck{ true };

    /* 20210804 wp?? 配置文件中指定当前通讯地址收到的数据是往哪个模块发送
     *  但是接收的函数是固定的
     *  正常的数据 接收是 AcceptDeviceMessage
     *  错误的数据 接收是 AcceptErrorDeviceMessage
     * 20211104 wp?? 更改一下这里的接收模块改为配置的模块+线程号
     * 线程号和下面的线程数量对应从1开始
     * 例:AcceptDeviceMessage1
     */
    QString recvDataDesModule; /* 接收到数据发送的目标模块 */
    qint32 channelNo{ 1 };        /* 解析通道号从1开始 */
};
struct CommunicationInfoMap
{
    qint32 channelNum{ 1 }; /* 通道数量 最小为1 */
    QMap<quint32, CommunicationInfo> infoMap;
};
struct CommunicationInfoList
{
    int channelNum{ 1 }; /* 通道数量 最小为1 */
    QList<CommunicationInfo> infoList;
};

// using CommunicationInfoMap = QMap<quint32, CommunicationInfo>;
// using CommunicationInfoList = QList<CommunicationInfo>;

Q_DECLARE_METATYPE(CommunicationInfo);
#endif  // COMMUNICATIONINFO_H
