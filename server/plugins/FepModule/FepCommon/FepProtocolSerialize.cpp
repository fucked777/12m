#include "FepProtocolSerialize.h"
#include <QHostAddress>
#include <QtEndian>
#include <cstring>
#include <string>

/* 协议全部使用的是大端 */
template<typename T>
static void localToBigEndian(void* pos, T value)
{
    value = qToBigEndian<T>(value);
    std::memcpy(pos, &value, sizeof(T));
}
template<typename T>
static T bigToLocalEndian(const void* pos)
{
    T tempValue{};
    std::memcpy(&tempValue, pos, sizeof(T));
    return qFromBigEndian<T>(tempValue);
}

QByteArray FepProtocolSerialize::toByteArray(QString& errMsg, const FepSendREQ& data)
{
    static_assert(sizeof(data.fileLen) == ProtocolLen::fepFileContentLen, "协议中文件内容长度是4个字节");
    constexpr auto packLen = ProtocolLen::fepPackSendREQ;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(data.fepPackType);
    pos += ProtocolLen::fepTypeID;

    /* 2文件名长度 */
    if (data.fileName.isEmpty())
    {
        errMsg = "Fep:文件名不能为空";
        return QByteArray();
    }
    /* 协议中文件名长度固定为64字节 */
    auto byteName = data.fileName.toUtf8();
    auto size = byteName.size();
    if (size > ProtocolLen::fepFileName)
    {
        errMsg = "Fep:文件名过长";
        return QByteArray();
    }

    std::memcpy(&(buffer[pos]), byteName.data(), size);
    pos += ProtocolLen::fepFileName;

    /* 3文件长度 对方接收是大端 */
    localToBigEndian<decltype(data.fileLen)>(&(buffer[pos]), data.fileLen);

    return QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
}
bool FepProtocolSerialize::fromByteArray(QString& errMsg, FepSendREQ& data, const QByteArray& array)
{
    constexpr auto packLen = ProtocolLen::fepPackSendREQ;
    if (array.size() < packLen)
    {
        errMsg = QString("Fep:数据包长度与协议不符合,发送请求协议长度:%1,当前长度:%2").arg(packLen).arg(array.size());
        return false;
    }
    auto constArray = array.constData();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = constArray[pos];
    if (tempType != static_cast<char>(data.fepPackType))
    {
        errMsg = "Fep:数据包发送请求协议不符";
        return false;
    }
    pos += ProtocolLen::fepTypeID;

    /* 2文件名 */
    auto tempFileNameArray = QByteArray(&(constArray[pos]), ProtocolLen::fepFileName);
    auto endIndex = tempFileNameArray.indexOf('\0');
    if (-1 == endIndex)
    {
        /* 没找到 */
        data.fileName = QString::fromUtf8(tempFileNameArray);
    }
    else
    {
        data.fileName = QString::fromUtf8(tempFileNameArray.left(endIndex));
    }
    if (data.fileName.isEmpty())
    {
        errMsg = "Fep:数据包解析失败,文件名为空";
        return false;
    }
    pos += ProtocolLen::fepFileName;

    /* 3文件内容长度 */
    static_assert(sizeof(data.fileLen) == ProtocolLen::fepFileContentLen, "协议中文件内容占用4个字节");
    data.fileLen = bigToLocalEndian<quint32>(&(constArray[pos]));
    return true;
}

QByteArray FepProtocolSerialize::toByteArray(QString& errMsg, const FepAnserREQ& data)
{
    constexpr auto packLen = ProtocolLen::fepPackAnserREQ;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(data.fepPackType);
    pos += ProtocolLen::fepTypeID;

    /* 2文件名长度 */
    if (data.fileName.isEmpty())
    {
        errMsg = "Fep:文件名不能为空";
        return QByteArray();
    }
    /* 协议中文件名长度固定为64字节 */
    auto byteName = data.fileName.toUtf8();
    auto size = byteName.size();
    if (size > ProtocolLen::fepFileName)
    {
        errMsg = "Fep:文件名过长";
        return QByteArray();
    }

    std::memcpy(&(buffer[pos]), byteName.data(), size);
    pos += ProtocolLen::fepFileName;

    /* 3数据单元号 */
    static_assert(sizeof(data.unitNum) == ProtocolLen::fepDataUnitID, "协议中数据单元号是4个字节");
    localToBigEndian<decltype(data.unitNum)>(&(buffer[pos]), data.unitNum);
    pos += ProtocolLen::fepDataUnitID;

    /* 4文件标识 */
    static_assert(sizeof(data.fileID) == ProtocolLen::fepFileID, "协议中文件标识是2个字节");
    localToBigEndian<decltype(data.fileID)>(&(buffer[pos]), data.fileID);

    return QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
}
bool FepProtocolSerialize::fromByteArray(QString& errMsg, FepAnserREQ& data, const QByteArray& array)
{
    constexpr auto packLen = ProtocolLen::fepPackAnserREQ;
    if (array.size() < packLen)
    {
        errMsg = QString("Fep:数据包长度与协议不符合,发送请求协议长度:%1,当前长度:%2").arg(packLen).arg(array.size());
        return false;
    }
    auto constArray = array.constData();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = constArray[pos];
    if (tempType != static_cast<char>(data.fepPackType))
    {
        errMsg = "Fep:数据包请求应答协议不符";
        return false;
    }
    pos += ProtocolLen::fepTypeID;

    /* 2文件名 */
    auto tempFileNameArray = QByteArray(&(constArray[pos]), ProtocolLen::fepFileName);
    auto endIndex = tempFileNameArray.indexOf('\0');
    if (-1 == endIndex)
    {
        /* 没找到 */
        data.fileName = QString::fromUtf8(tempFileNameArray);
    }
    else
    {
        data.fileName = QString::fromUtf8(tempFileNameArray.left(endIndex));
    }
    if (data.fileName.isEmpty())
    {
        errMsg = "Fep:数据包解析失败,文件名为空";
        return false;
    }
    pos += ProtocolLen::fepFileName;

    /* 3数据单元号 */
    data.unitNum = bigToLocalEndian<decltype(data.unitNum)>(&(constArray[pos]));
    pos += ProtocolLen::fepDataUnitID;

    /* 4文件标识 */
    data.fileID = bigToLocalEndian<decltype(data.fileID)>(&(constArray[pos]));
    return true;
}

QByteArray FepProtocolSerialize::toByteArray(QString& /*errMsg*/, const FepFinish& data)
{
    constexpr auto packLen = ProtocolLen::fepPackFinish;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(data.fepPackType);
    pos += ProtocolLen::fepTypeID;

    /* 2文件标识 */
    localToBigEndian<decltype(data.fileID)>(&(buffer[pos]), data.fileID);

    return QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
}
bool fromByteArray(QString& errMsg, FepFinish& data, const QByteArray& array)
{
    constexpr auto packLen = ProtocolLen::fepPackFinish;
    if (array.size() < packLen)
    {
        errMsg = QString("Fep:数据包长度与协议不符合,发送请求协议长度:%1,当前长度:%2").arg(packLen).arg(array.size());
        return false;
    }
    auto constArray = array.constData();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = constArray[pos];
    if (tempType != static_cast<char>(data.fepPackType))
    {
        errMsg = "Fep:数据包请求应答协议不符";
        return false;
    }
    pos += ProtocolLen::fepTypeID;

    /* 2文件标识 */
    data.fileID = bigToLocalEndian<decltype(data.fileID)>(&(constArray[pos]));
    return true;
}

QByteArray FepProtocolSerialize::toByteArray(QString& errMsg, const FepData& data)
{
    constexpr auto packLen = ProtocolLen::fepPackMinData;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(data.fepPackType);
    pos += ProtocolLen::fepTypeID;

    /* 2数据单元号 */
    localToBigEndian<decltype(data.unitNum)>(&(buffer[pos]), data.unitNum);
    pos += ProtocolLen::fepDataUnitID;

    /* 3文件标识 */
    localToBigEndian<decltype(data.fileID)>(&(buffer[pos]), data.fileID);
    pos += ProtocolLen::fepFileID;

    /* 4数据 */
    if (data.data.size() > ProtocolLen::fepDataMaxLen)
    {
        errMsg = "Fep:当前发送数据过长";
        return QByteArray();
    }

    QByteArray array;
    array = QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
    array.append(data.data);
    return array;
}
bool FepProtocolSerialize::fromByteArrayExcludeData(QString& errMsg, FepData& data, const QByteArray& array)
{
    constexpr auto packLen = ProtocolLen::fepPackMinData;
    if (array.size() < packLen)
    {
        errMsg = QString("Fep:数据包长度与协议不符合,发送请求协议长度:%1,当前长度:%2").arg(packLen).arg(array.size());
        return false;
    }
    auto constArray = array.constData();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = constArray[pos];
    if (tempType != static_cast<char>(data.fepPackType))
    {
        errMsg = "Fep:数据包请求应答协议不符";
        return false;
    }
    pos += ProtocolLen::fepTypeID;

    /* 2数据单元号 */
    data.unitNum = bigToLocalEndian<decltype(data.unitNum)>(&(constArray[pos]));
    pos += ProtocolLen::fepDataUnitID;

    /* 3文件标识 */
    data.fileID = bigToLocalEndian<decltype(data.fileID)>(&(constArray[pos]));
    // pos += ProtocolLen::fepFileID;

    /* 4.数据 将剩下的数据赋值 */
    // data.data = array.mid(pos);
    return true;
}
bool FepProtocolSerialize::fromByteArray(QString& errMsg, FepData& data, const QByteArray& array)
{
    constexpr auto packLen = ProtocolLen::fepPackMinData;
    if (array.size() < packLen)
    {
        errMsg = QString("Fep:数据包长度与协议不符合,发送请求协议长度:%1,当前长度:%2").arg(packLen).arg(array.size());
        return false;
    }
    auto constArray = array.constData();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = constArray[pos];
    if (tempType != static_cast<char>(data.fepPackType))
    {
        errMsg = "Fep:数据包请求应答协议不符";
        return false;
    }
    pos += ProtocolLen::fepTypeID;

    /* 2数据单元号 */
    data.unitNum = bigToLocalEndian<decltype(data.unitNum)>(&(constArray[pos]));
    pos += ProtocolLen::fepDataUnitID;

    /* 3文件标识 */
    data.fileID = bigToLocalEndian<decltype(data.fileID)>(&(constArray[pos]));
    pos += ProtocolLen::fepFileID;

    /* 4.数据 将剩下的数据赋值 */
    data.data = array.mid(pos);
    return true;
}

QByteArray FepProtocolSerialize::toByteArray(QString& /*errMsg*/, const RECPProtocolHead& data)
{
    constexpr auto packLen = ProtocolLen::recpHead;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1 包类型 */
    buffer[pos] = static_cast<quint8>(data.recpPackType);
    pos += ProtocolLen::recpTypeID;

    /* 2 源地址 */
    auto ipv4Addr = QHostAddress(data.srcIP).toIPv4Address();
    static_assert(sizeof(ipv4Addr) == ProtocolLen::recpSrcAddr, "协议中源IP地址是4个字节");
    localToBigEndian<decltype(ipv4Addr)>(&(buffer[pos]), ipv4Addr);
    pos += ProtocolLen::recpSrcAddr;

    /* 3 序号 */
    static_assert(sizeof(data.sn) == ProtocolLen::recpSN, "协议中序号是4个字节");
    localToBigEndian<qint16>(&(buffer[pos]), data.sn);
    pos += ProtocolLen::recpSN;

    /* 4 保留位 */
    static_assert(sizeof(data.retain) == ProtocolLen::recpRetain, "协议保留位是4个字节");
    std::memcpy(&(buffer[pos]), &(data.retain[0]), sizeof(data.retain));
    pos += ProtocolLen::recpRetain;

    /* 5 摘要长度 */
    static_assert(sizeof(data.abstractLength) == ProtocolLen::recpAbstractLength, "协议中摘要长度是4个字节");
    localToBigEndian<qint16>(&(buffer[pos]), data.abstractLength);
    pos += ProtocolLen::recpAbstractLength;

    /* 6 摘要 */
    static_assert(sizeof(data.abstract) == ProtocolLen::recpAbstract, "协议摘要是56个字节");
    std::memcpy(&(buffer[pos]), &(data.abstract[0]), sizeof(data.abstract));

    return QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
}
bool FepProtocolSerialize::fromByteArray(QString& errMsg, RECPProtocolHead& data, const QByteArray& array)
{
    constexpr auto packLen = ProtocolLen::recpHead;
    if (array.size() < packLen)
    {
        errMsg = QString("Fep:数据包长度与协议不符合,发送请求协议长度:%1,当前长度:%2").arg(packLen).arg(array.size());
        return false;
    }
    auto constArray = array.constData();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    data.recpPackType = RECPPackType(constArray[pos]);
    pos += ProtocolLen::recpTypeID;

    /* 2 源地址 */
    auto addr = bigToLocalEndian<quint32>(&(constArray[pos]));
    data.srcIP = QHostAddress(addr).toString();
    pos += ProtocolLen::recpSrcAddr;

    /* 3 序号 */
    data.sn = bigToLocalEndian<decltype(data.sn)>(&(constArray[pos]));
    pos += ProtocolLen::recpSN;

    /* 4 保留位 */
    std::memcpy(&(data.retain[0]), &(constArray[pos]), sizeof(data.retain));
    pos += ProtocolLen::recpRetain;

    /* 5 摘要长度 */
    data.abstractLength = bigToLocalEndian<decltype(data.abstractLength)>(&(constArray[pos]));
    pos += ProtocolLen::recpAbstractLength;

    /* 6 摘要 */
    std::memcpy(&(data.abstract[0]), &(constArray[pos]), sizeof(data.abstract));
    return true;
}

FepRecvContent::FepRecvContent() {}
FepRecvContent::~FepRecvContent()
{
    if (file.isOpen())
    {
        file.flush();
        file.close();
    }
}

void FepRecvContent::reset()
{
    srcIP.clear();    /* ip地址 */
    curUnitNum = 0;   /* 当前预计的单元号 */
    recvFileSize = 0; /* 当前已接收文件的大小 */
    srcPort = 0;      /* 端口号 */
    state = FepRecvState::Wait;
    workDir.clear(); /* 工作目录 */
    recvFileInfo = FepRecvFileInfo();

    if (file.isOpen())
    {
        file.flush();
        file.close();
    }
}
