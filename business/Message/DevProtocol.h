#ifndef DEVPROTOCOL_H
#define DEVPROTOCOL_H
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>
#include <QTime>
#include <QVariant>
#include <type_traits>

/* 不启用兼容就是直接使用新的协议,没有系统号,兼容的话就有系统号 */
#define COMPATIBLE

// 命令类型枚举
enum class DevMsgType : quint32
{
    Unknown = 0,                         //未知协议
    ProcessControlCmd = 0x0000F001,      //过程控制命令
    UnitParameterSetCmd = 0x0000F002,    //单元参数设置命令
    GroupParamSetCmd = 0x0000F003,       //组参数设置命令
    ControlCmdResponse = 0x0000F100,     //控制命令响应
    StatusQueryCmd = 0x0000F004,         //状态查询命令
    ControlResultReport = 0x0000F101,    //控制结果上报
    ExtensionStatusReport = 0x0000F104,  //分机状态信息上报
    ParameterSet = 0x0000F000,           //参数设置命令（保留格式，暂不使用）
};
// 控制命令响应枚举
enum class ControlCmdResponseType : quint32
{
    Success = 0,             // 0：正常接收并执行
    SubControlRejected,      // 1：分控拒收
    FrameFormatError,        // 2：帧格式错误（未知命令）
    ControllObjectNotExist,  // 3：被控对象不存在
    ParameterError,          // 4：参数错误
    ConditionsNotAvailable,  // 5：条件不具备
    UnknowError,             // 6：未知原因失败
    UnitDataLengthOverrun    // 7：目标单元数据长度超限
};

/* 协议头 */
/* 地址数据类型 */
struct MessageAddress
{
    quint8 baseNumb{ 0 };       //基地ID 1
    quint8 stationNumb{ 0 };    //站ID   1
    quint8 equipmentNumb{ 0 };  //设备号  1
#ifdef COMPATIBLE
    quint8 systemNumb{ 0 };  //系统ID  1
    quint8 deviceNumb{ 0 };  //设备ID  1
#else
    quint8 extenName{ 0 };  //分机名称 1
#endif
    quint8 extenNumb{ 0 };  //分机ID  1
    quint8 reserved[3]{};   //保留   3

    /* 当前的地址是否是有效的 */
    bool isValid() { return baseNumb != 0; }
};

/* 版本号 */
struct Version
{
    /* 高字节表示主版本号，低字节分为两段，每段4位，表示副版本号，例如V2.1.3表示为0213H */
    uchar mainVersion{ 0 };    //主版本号 占8个bit
    uchar subVersionOne{ 0 };  //副版本号1 占4个bit
    uchar subVersionTwo{ 0 };  //副版本号2 占4个bit
};

/* DevProtocolHeader DevMessageHeader是一样的一个是解析了的一个未解析 */
struct DevMessageHeader
{
    MessageAddress sourceAddr;                 /* 源地址 */
    MessageAddress targetAddr;                 /* 目的地址 */
    DevMsgType msgType{ DevMsgType::Unknown }; /* 信息类别表示传输的信息类型 */
    Version version;                           /* 版本号 */
    quint16 reserve{ 0 };                      /* 目前填0，暂时第一个字节用于区分中心体分控软件和系统监控软件命令 */
    QDate date;                                /* 消息发送的日期 */
    QTime time;                                /* 消息发送的时间 */
    qint32 dataLen{ 0 };                       /* 数据域字节长度 */

    /* 当前的消息头是否是有效的 */
    /* 故意用的&  只做一次判断 */
    bool isValid() { return (sourceAddr.isValid() & targetAddr.isValid() & (msgType != DevMsgType::Unknown)); }
};

struct DevProtocolHeader
{
    /* 警告: 这里的类型不能随意更改类型的字节长度与协议是一致的,修改会会导致异常 */

    quint64 sid{ 0 };     /* 信源地址表示此帧信息发送者 8B */
    quint64 did{ 0 };     /* 目的地址表示此帧信息接收者 8B */
    quint32 bid{ 0 };     /* 信息类别表示传输的信息类型 4B */
    quint16 ver{ 0 };     /* 版本号 2B */
    quint16 reserve{ 0 }; /* 目前填0，暂时第一个字节用于区分中心体分控软件和系统监控软件命令 2B */
    quint32 date{ 0 };    /* 生成本帧信息的日期 4B */
    quint32 time{ 0 };    /* 生成本帧信息的时间 4B */
    qint32 dataLen{ 0 };  /* 以字节为单位表示数据域的实际长度 4B */

    /* 当前的消息头是否是有效的 */
    /* 故意用的&  只做一次判断 */
    bool isValid() { return ((sid != 0) & (did != 0) & (bid != 0)); }
};

#ifndef COMPATIBLE
/* 协议地址的长度 */
using MessageAddressLength =
    std::integral_constant<int, sizeof(MessageAddress::baseNumb) + sizeof(MessageAddress::stationNumb) + sizeof(MessageAddress::equipmentNumb) +
                                    sizeof(MessageAddress::extenName) + sizeof(MessageAddress::extenNumb) + sizeof(MessageAddress::reserved)>;
#else
/* 协议地址的长度 */
using MessageAddressLength =
    std::integral_constant<int, sizeof(MessageAddress::baseNumb) + sizeof(MessageAddress::stationNumb) + sizeof(MessageAddress::equipmentNumb) + 1 +
                                    sizeof(MessageAddress::extenNumb) + sizeof(MessageAddress::reserved)>;
#endif
/* 版本号只占用1个字节 */
using VersionLength = std::integral_constant<int, 2>;

using DevProtocolSIDLength = std::integral_constant<int, sizeof(DevProtocolHeader::sid)>;
using DevProtocolDIDLength = std::integral_constant<int, sizeof(DevProtocolHeader::did)>;
using DevProtocolBIDLength = std::integral_constant<int, sizeof(DevProtocolHeader::bid)>;
using DevProtocolVerLength = std::integral_constant<int, sizeof(DevProtocolHeader::ver)>;
using DevProtocolReserveLength = std::integral_constant<int, sizeof(DevProtocolHeader::reserve)>;
using DevProtocolDateLength = std::integral_constant<int, sizeof(DevProtocolHeader::date)>;
using DevProtocolTimeLength = std::integral_constant<int, sizeof(DevProtocolHeader::time)>;
using DevProtocolDataLenLength = std::integral_constant<int, sizeof(DevProtocolHeader::dataLen)>;

/* 此索引是从1开始的索引 */
using DevProtocolSIDIndex = std::integral_constant<int, 0>;
using DevProtocolDIDIndex = std::integral_constant<int, DevProtocolSIDIndex::value + DevProtocolSIDLength::value>;
using DevProtocolBIDIndex = std::integral_constant<int, DevProtocolDIDIndex::value + DevProtocolDIDLength::value>;
using DevProtocolVerIndex = std::integral_constant<int, DevProtocolBIDIndex::value + DevProtocolBIDLength::value>;
using DevProtocolReserveIndex = std::integral_constant<int, DevProtocolVerIndex::value + DevProtocolVerLength::value>;
using DevProtocolDateIndex = std::integral_constant<int, DevProtocolReserveIndex::value + DevProtocolReserveLength::value>;
using DevProtocolTimeIndex = std::integral_constant<int, DevProtocolDateIndex::value + DevProtocolDateLength::value>;
using DevProtocolDataLenIndex = std::integral_constant<int, DevProtocolTimeIndex::value + DevProtocolTimeLength::value>;

/* 协议头总长 */
using DevProtocolLength =
    std::integral_constant<int, DevProtocolSIDLength::value + DevProtocolDIDLength::value + DevProtocolBIDLength::value +
                                    DevProtocolVerLength::value + DevProtocolReserveLength::value + DevProtocolDateLength::value +
                                    DevProtocolTimeLength::value + DevProtocolDataLenLength::value>;

class DevProtocolEnumHelper
{
public:
    static QString devMsgTypeToStr(DevMsgType type)
    {
        switch (type)
        {
        case DevMsgType::Unknown:
        {
            return QString("Unknow");
        }
        case DevMsgType::ProcessControlCmd:
        {
            return QString("ProcessControlCmd");
        }
        case DevMsgType::UnitParameterSetCmd:
        {
            return QString("UnitParameterSetCmd");
        }
        case DevMsgType::GroupParamSetCmd:
        {
            return QString("GroupParamSetCmd");
        }
        case DevMsgType::ControlCmdResponse:
        {
            return QString("ControlCmdResponse");
        }
        case DevMsgType::StatusQueryCmd:
        {
            return QString("StatusQueryCmd");
        }
        case DevMsgType::ControlResultReport:
        {
            return QString("ControlResultReport");
        }
        case DevMsgType::ExtensionStatusReport:
        {
            return QString("ExtensionStatusReport");
        }
        case DevMsgType::ParameterSet:
        {
            return QString("ParameterSet");
        }
        }
        return QString("Unknow");
    }
    static DevMsgType devMsgTypeFromStr(const QString& strValue)
    {
        auto normalize = strValue.toUpper();

        if (normalize == "UnKnow")
        {
            return (DevMsgType::Unknown);
        }
        else if (strValue == "ProcessControlCmd")
        {
            return (DevMsgType::ProcessControlCmd);
        }
        else if (strValue == "UnitParameterSetCmd")
        {
            return (DevMsgType::UnitParameterSetCmd);
        }
        else if (strValue == "GroupParamSetCmd")
        {
            return (DevMsgType::GroupParamSetCmd);
        }
        else if (strValue == "ControlCmdResponse")
        {
            return (DevMsgType::ControlCmdResponse);
        }
        else if (strValue == "StatusQueryCmd")
        {
            return (DevMsgType::StatusQueryCmd);
        }
        else if (strValue == "ControlResultReport")
        {
            return (DevMsgType::ControlResultReport);
        }
        else if (strValue == "ExtensionStatusReport")
        {
            return (DevMsgType::ExtensionStatusReport);
        }
        else if (strValue == "ParameterSet")
        {
            return (DevMsgType::ParameterSet);
        }
        return (DevMsgType::Unknown);
    }
    static QString devMsgTypeToDescStr(DevMsgType type)
    {
        switch (type)
        {
        case DevMsgType::Unknown:
        {
            return QString("未知协议");
        }
        case DevMsgType::ProcessControlCmd:
        {
            return QString("过程控制命令");
        }
        case DevMsgType::UnitParameterSetCmd:
        {
            return QString("单元参数设置命令");
        }
        case DevMsgType::GroupParamSetCmd:
        {
            return QString("组参数设置命令");
        }
        case DevMsgType::ControlCmdResponse:
        {
            return QString("控制命令响应");
        }
        case DevMsgType::StatusQueryCmd:
        {
            return QString("状态查询命令");
        }
        case DevMsgType::ControlResultReport:
        {
            return QString("控制结果上报");
        }
        case DevMsgType::ExtensionStatusReport:
        {
            return QString("分机状态信息上报");
        }
        case DevMsgType::ParameterSet:
        {
            return QString("参数设置命(保留格式，暂不使用)");
        }
        }
        return QString("未知协议");
    }

    static QString controlCmdResponseToDescStr(ControlCmdResponseType type)
    {
        switch (type)
        {
        case ControlCmdResponseType::Success:
        {
            return QString("正常接收并执行");
        }
        case ControlCmdResponseType::SubControlRejected:
        {
            return QString("分控拒收");
        }
        case ControlCmdResponseType::FrameFormatError:
        {
            return QString("帧格式错误（未知命令）");
        }
        case ControlCmdResponseType::ControllObjectNotExist:
        {
            return QString("被控对象不存在");
        }
        case ControlCmdResponseType::ParameterError:
        {
            return QString("参数错误");
        }
        case ControlCmdResponseType::ConditionsNotAvailable:
        {
            return QString("条件不具备");
        }
        case ControlCmdResponseType::UnknowError:
        {
            return QString("未知原因失败");
        }
        case ControlCmdResponseType::UnitDataLengthOverrun:
        {
            return QString("目标单元数据长度超限");
        }
        }
        return QString("未知上报响应");
    }
};

#endif  // DEVPROTOCOL_H
