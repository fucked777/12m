#include "DevProtocolSerialize.h"
#include "JsonHelper.h"
#include "Utility.h"
#include <CConverter.h>
#include <QDataStream>
#include <QDebug>

QString DevMsgTypeHelper::toDescStr(DevMsgType type)
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

QString DevMsgTypeHelper::toStr(DevMsgType type)
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
DevMsgType DevMsgTypeHelper::fromStr(const QString& strValue)
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
QByteArray& operator<<(QByteArray& self, const DevProtocolHeader& value)
{
    self += CConverter::toByteArray<typename std::decay<decltype(value.sid)>::type>(value.sid);
    self += CConverter::toByteArray<typename std::decay<decltype(value.did)>::type>(value.did);
    self += CConverter::toByteArray<typename std::decay<decltype(value.bid)>::type>(value.bid);
    self += CConverter::toByteArray<typename std::decay<decltype(value.ver)>::type>(value.ver);
    self += CConverter::toByteArray<typename std::decay<decltype(value.reserve)>::type>(value.reserve);
    self += CConverter::toByteArray<typename std::decay<decltype(value.date)>::type>(value.date);
    self += CConverter::toByteArray<typename std::decay<decltype(value.time)>::type>(value.time);
    self += CConverter::toByteArray<typename std::decay<decltype(value.dataLen)>::type>(value.dataLen);
    return self;
}

QByteArray& operator>>(QByteArray& self, DevProtocolHeader& value)
{
    if (self.size() < DevProtocolLength::value)
    {
        return self;
    }
    auto array = self.left(DevProtocolLength::value);
    // self = self.mid(0, DevProtocolHeader::Length::value);

    int index = 0;
    value.sid = CConverter::fromByteArray<typename std::decay<decltype(value.sid)>::type>(array.mid(index, sizeof(value.sid)));
    index += sizeof(value.sid);

    value.did = CConverter::fromByteArray<typename std::decay<decltype(value.did)>::type>(array.mid(index, sizeof(value.did)));
    index += sizeof(value.did);

    value.bid = CConverter::fromByteArray<typename std::decay<decltype(value.bid)>::type>(array.mid(index, sizeof(value.bid)));
    index += sizeof(value.bid);

    value.ver = CConverter::fromByteArray<typename std::decay<decltype(value.ver)>::type>(array.mid(index, sizeof(value.ver)));
    index += sizeof(value.ver);

    value.reserve = CConverter::fromByteArray<typename std::decay<decltype(value.reserve)>::type>(array.mid(index, sizeof(value.reserve)));
    index += sizeof(value.reserve);

    value.date = CConverter::fromByteArray<typename std::decay<decltype(value.date)>::type>(array.mid(index, sizeof(value.date)));
    index += sizeof(value.date);

    value.time = CConverter::fromByteArray<typename std::decay<decltype(value.time)>::type>(array.mid(index, sizeof(value.time)));
    index += sizeof(value.time);

    value.dataLen = CConverter::fromByteArray<typename std::decay<decltype(value.dataLen)>::type>(array.mid(index, sizeof(value.dataLen)));

    return self;
}

QByteArray& operator<<(QByteArray& self, const DevMessageHeader& value)
{
    self << value.sourceAddr;
    self << value.targetAddr;
    static_assert(sizeof(value.msgType) == sizeof(DevProtocolHeader::bid), "DevMessageHeader与DevProtocolHeader 消息类型长度不符");
    self += CConverter::toByteArray<typename std::decay<decltype(value.msgType)>::type>(value.msgType);

    self << value.version;
    static_assert(sizeof(value.reserve) == sizeof(DevProtocolHeader::reserve), "DevMessageHeader与DevProtocolHeader 保留字长度不符");
    self += CConverter::toByteArray<typename std::decay<decltype(value.reserve)>::type>(value.reserve);

    if (value.date.isValid())
    {
        auto data = value.date.toString("yyyyMMdd").toUInt(nullptr, 16);
        static_assert(sizeof(data) == sizeof(DevProtocolHeader::date), "DevMessageHeader与DevProtocolHeader 日期长度不符");
        self += CConverter::toByteArray<typename std::decay<decltype(data)>::type>(data);

        //  int year = value.date.year();
        //  int month = value.date.month();
        //  int day = value.date.day();

        //  int val = ((year / 1000 % 10) << 28) | ((year / 100 % 10) << 24) | ((year / 10 % 10) << 20) | ((year / 1 % 10) << 16) |
        //            ((month / 10 % 10) << 12) | ((month / 1 % 10) << 8) | ((day / 10 % 10) << 4) | (day / 1 % 10);

        //  static_assert(sizeof(val) == sizeof(DevProtocolHeader::date), "DevMessageHeader与DevProtocolHeader 日期长度不符");
        //  self += CConverter::toByteArray<typename std::decay<decltype(val)>::type>(val);
    }
    else
    {
        self += QByteArray(static_cast<int>(sizeof(DevProtocolHeader::date)), 0);
    }

    int time = value.time.msecsSinceStartOfDay();
    static_assert(sizeof(time) == sizeof(DevProtocolHeader::time), "DevMessageHeader与DevProtocolHeader 时间长度不符");
    self += CConverter::toByteArray<typename std::decay<decltype(time)>::type>(time);

    static_assert(sizeof(value.dataLen) == sizeof(DevProtocolHeader::dataLen), "DevMessageHeader与DevProtocolHeader 数据长度长度不符");
    self += CConverter::toByteArray<typename std::decay<decltype(value.dataLen)>::type>(value.dataLen);

    return self;
}
QByteArray& operator>>(QByteArray& self, DevMessageHeader& value)
{
    if (self.size() < DevProtocolLength::value)
    {
        return self;
    }
    auto array = self.left(DevProtocolLength::value);
    // self = self.mid(0, DevProtocolHeader::Length::value);

    int index = 0;
    auto temp = array.mid(index, sizeof(DevProtocolHeader::sid));
    index += sizeof(DevProtocolHeader::sid);
    temp >> value.sourceAddr;

    temp = array.mid(index, sizeof(DevProtocolHeader::did));
    index += sizeof(DevProtocolHeader::did);
    temp >> value.targetAddr;

    value.msgType = CConverter::fromByteArray<typename std::decay<decltype(value.msgType)>::type>(array.mid(index, sizeof(DevProtocolHeader::bid)));
    index += sizeof(DevProtocolHeader::bid);

    temp = array.mid(index, sizeof(DevProtocolHeader::ver));
    index += sizeof(DevProtocolHeader::ver);
    temp >> value.version;

    value.reserve =
        CConverter::fromByteArray<typename std::decay<decltype(value.reserve)>::type>(array.mid(index, sizeof(DevProtocolHeader::reserve)));
    index += sizeof(DevProtocolHeader::reserve);

    auto date =
        CConverter::fromByteArray<typename std::decay<decltype(DevProtocolHeader::date)>::type>(array.mid(index, sizeof(DevProtocolHeader::date)));
    index += sizeof(DevProtocolHeader::date);

    //    auto year = static_cast<int>(((date >> 28) & 0xF) * 1000 + ((date >> 24) & 0xF) * 100 + ((date >> 20) & 0xF) * 10 + ((date >> 16) & 0xF));
    //    int month = static_cast<int>(((date >> 12) & 0xF) * 10 + ((date >> 8) & 0xF));
    //    int day = static_cast<int>(((date >> 4) & 0xF) * 10 + (date & 0xF));

    value.date = QDate::fromString(QString::number(date, 16), "yyyyMMdd");

    auto time =
        CConverter::fromByteArray<typename std::decay<decltype(DevProtocolHeader::time)>::type>(array.mid(index, sizeof(DevProtocolHeader::time)));
    value.time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(time));
    index += sizeof(DevProtocolHeader::time);

    value.dataLen = CConverter::fromByteArray<typename std::decay<decltype(DevProtocolHeader::dataLen)>::type>(
        array.mid(index, sizeof(DevProtocolHeader::dataLen)));

    return self;
}
QByteArray& operator<<(QByteArray& self, const MessageAddress& value)
{
    QByteArray res;
    res.resize(8);
    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = value.extenNumb;
#ifndef COMPATIBLE
    res[4] = value.extenName;
#else
    res[4] = static_cast<qint8>(value.systemNumb * 0x10 + value.deviceNumb);
#endif

    res[5] = value.equipmentNumb;
    res[6] = value.stationNumb;
    res[7] = value.baseNumb;
    self.append(res);
    return self;
}

QByteArray& operator>>(QByteArray& self, MessageAddress& value)
{
    if (self.size() < MessageAddressLength::value)
    {
        return self;
    }
    auto array = self.left(MessageAddressLength::value);
    // self = self.mid(0, MessageAddress::Length::value);

    /* 这三个是保留的 */
    (value.reserved)[0] = array[0];
    (value.reserved)[1] = array[1];
    (value.reserved)[2] = array[2];

    value.extenNumb = array[3];
#ifndef COMPATIBLE
    value.extenName = array[4];
#else
    auto temp = (uchar)array[4];
    value.systemNumb = temp / 0x10;
    value.deviceNumb = temp % 0x10;
#endif
    value.equipmentNumb = array[5];
    value.stationNumb = array[6];
    value.baseNumb = array[7];

    return self;
}
QByteArray& operator<<(QByteArray& self, const Version& value)
{
    QByteArray version;
    version.resize(2);
    version[1] = value.mainVersion;
    version[0] = (value.subVersionOne << 4) | (value.subVersionTwo);
    self += version;
    return self;
}
QByteArray& operator>>(QByteArray& self, Version& value)
{
    if (self.size() < VersionLength::value)
    {
        return self;
    }
    auto array = self.left(VersionLength::value);
    // self = self.mid(0, Version::Length::value);

    value.mainVersion = array[1];
    auto low = array[0];
    value.subVersionOne = (low >> 4);
    value.subVersionTwo = (low & 0xF);
    return self;
}

quint64 ProtocolHeaderBit::sidRaw(const QByteArray& array)
{
    /*0 8*/
    static constexpr auto index = DevProtocolSIDIndex::value;
    static constexpr auto len = DevProtocolSIDLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint64>(array.mid(index, len));
}
quint64 ProtocolHeaderBit::didRaw(const QByteArray& array)
{
    /*8 8*/
    static constexpr auto index = DevProtocolDIDIndex::value;
    static constexpr auto len = DevProtocolDIDLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint64>(array.mid(index, len));
}
quint32 ProtocolHeaderBit::bidRaw(const QByteArray& array)
{
    /*16 4*/
    static constexpr auto index = DevProtocolBIDIndex::value;
    static constexpr auto len = DevProtocolBIDLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint32>(array.mid(index, len));
}
quint16 ProtocolHeaderBit::verRaw(const QByteArray& array)
{
    /*20 2*/
    static constexpr auto index = DevProtocolVerIndex::value;
    static constexpr auto len = DevProtocolVerLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint16>(array.mid(index, len));
}
quint16 ProtocolHeaderBit::reserveRaw(const QByteArray& array)
{
    /*22 2*/
    static constexpr auto index = DevProtocolReserveIndex::value;
    static constexpr auto len = DevProtocolReserveLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint16>(array.mid(index, len));
}
quint32 ProtocolHeaderBit::dateRaw(const QByteArray& array)
{
    /*24 4*/
    static constexpr auto index = DevProtocolDateIndex::value;
    static constexpr auto len = DevProtocolDateLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint32>(array.mid(index, len));
}
quint32 ProtocolHeaderBit::timeRaw(const QByteArray& array)
{
    /*28 4*/
    static constexpr auto index = DevProtocolTimeIndex::value;
    static constexpr auto len = DevProtocolTimeLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint32>(array.mid(index, len));
}
qint32 ProtocolHeaderBit::dataLenRaw(const QByteArray& array)
{
    /*32 4*/
    static constexpr auto index = DevProtocolDataLenIndex::value;
    static constexpr auto len = DevProtocolDataLenLength::value;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<qint32>(array.mid(index, len));
}
MessageAddress ProtocolHeaderBit::sidParse(const QByteArray& array)
{
    /*0 8*/
    static constexpr auto index = DevProtocolSIDIndex::value;
    static constexpr auto len = DevProtocolSIDLength::value;
    MessageAddress ret;
    if (array.size() < index + len)
    {
        return ret;
    }
    auto temp = array.mid(index, len);
    temp >> ret;
    return ret;
}

MessageAddress ProtocolHeaderBit::didParse(const QByteArray& array)
{
    /*8 8*/
    static constexpr auto index = DevProtocolDIDIndex::value;
    static constexpr auto len = DevProtocolDIDLength::value;
    MessageAddress ret;
    if (array.size() < index + len)
    {
        return ret;
    }
    auto temp = array.mid(index, len);
    temp >> ret;
    return ret;
}
DevMsgType ProtocolHeaderBit::bidParse(const QByteArray& array)
{
    /*16 4*/
    static constexpr auto index = DevProtocolBIDIndex::value;
    static constexpr auto len = DevProtocolBIDLength::value;
    return array.size() < index + len ? DevMsgType::Unknown : CConverter::fromByteArray<DevMsgType>(array.mid(index, len));
}
Version ProtocolHeaderBit::verParse(const QByteArray& array)
{
    /*20 2*/
    static constexpr auto index = DevProtocolVerIndex::value;
    static constexpr auto len = DevProtocolVerLength::value;
    Version ret;
    if (array.size() < index + len)
    {
        return ret;
    }
    auto temp = array.mid(index, len);
    temp >> ret;
    return ret;
}

quint16 ProtocolHeaderBit::reserveParse(const QByteArray& array) { return reserveRaw(array); }
QDate ProtocolHeaderBit::dateParse(const QByteArray& array)
{
    auto date = dataLenRaw(array);
    return QDate::fromString(QString::number(date, 16), "yyyyMMdd");
}
QTime ProtocolHeaderBit::timeParse(const QByteArray& array)
{
    auto time = timeRaw(array);
    return QTime::fromMSecsSinceStartOfDay(static_cast<int>(time));
}
qint32 ProtocolHeaderBit::dataLenParse(const QByteArray& array) { return dataLenRaw(array); }

quint32 ProtocolHeaderBit::didDevID(const QByteArray& array)
{
    /*3 2*/
    static constexpr auto index = 3 + 8;
    static constexpr auto len = 2;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint64>(array.mid(index, len));
}
quint32 ProtocolHeaderBit::sidDevID(const QByteArray& array)
{
    /*11 2*/
    static constexpr auto index = 3;
    static constexpr auto len = 2;
    return array.size() < index + len ? 0 : CConverter::fromByteArray<quint64>(array.mid(index, len));
}

QString ControlCmdResponseTypeHelper::toDescStr(ControlCmdResponseType type)
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
    }
    return QString("未知上报响应");
}
