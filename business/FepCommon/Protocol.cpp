#include "Protocol.h"
#include <QtEndian>
#include <cstring>
#include <string>

/* qt5.6loadLib 找不到符号 */

class Def
{
public:
    static constexpr int typeIDLen = 1;      /* 类型标识长度 */
    static constexpr int fileNameLen = 64;   /* 文件名长度 */
    static constexpr int fileContentLen = 4; /* 文件内容大小长度 */
    static constexpr int dataUnitIDLen = 4;  /* 数据单元号长度 */
    static constexpr int fileIDLen = 2;      /* 文件标识长度 */

    static constexpr int dataMaxLen = 4096; /* 数据内容长度 */
};

template<PackType type>
struct StateInfo
{
};
template<>
struct StateInfo<PackType::SendREQ>
{
    static constexpr auto packLen = Def::typeIDLen + Def::fileNameLen + Def::fileContentLen;
};
template<>
struct StateInfo<PackType::AnserREQ>
{
    static constexpr auto packLen = Def::typeIDLen + Def::fileNameLen + Def::dataUnitIDLen + Def::fileIDLen;
};
template<>
struct StateInfo<PackType::Finish>
{
    static constexpr auto packLen = Def::typeIDLen + Def::fileIDLen;
};
/* 数据包长度需要处理 */
template<>
struct StateInfo<PackType::Data>
{
    static constexpr auto packLen = Def::typeIDLen + Def::dataUnitIDLen + Def::fileIDLen + Def::dataMaxLen;
    static constexpr auto packMaxLen = packLen;
    /* 数据包是动态的,但是前面的标识字段是必须完整的 */
    static constexpr auto packMinLen = Def::typeIDLen + Def::dataUnitIDLen + Def::fileIDLen;
};

int DefLen::typeIDLen = Def::typeIDLen;           /* 类型标识长度 */
int DefLen::fileNameLen = Def::fileNameLen;       /* 文件名长度 */
int DefLen::fileContentLen = Def::fileContentLen; /* 文件内容大小长度 */
int DefLen::dataUnitIDLen = Def::dataUnitIDLen;   /* 数据单元号长度 */
int DefLen::fileIDLen = Def::fileIDLen;           /* 文件标识长度 */
int DefLen::dataMaxLen = Def::dataMaxLen;         /* 数据内容长度 */

int DefLen::packLenSendREQ = StateInfo<PackType::SendREQ>::packLen;
int DefLen::packLenAnserREQ = StateInfo<PackType::AnserREQ>::packLen;
int DefLen::packLenFinish = StateInfo<PackType::Finish>::packLen;
int DefLen::packLenData = StateInfo<PackType::Data>::packLen;
int DefLen::packMaxLenData = StateInfo<PackType::Data>::packLen;
int DefLen::packMinLenData = StateInfo<PackType::Data>::packMinLen;
int DefLen::maxBufferSize =
    qMax(StateInfo<PackType::SendREQ>::packLen,
         qMax(StateInfo<PackType::AnserREQ>::packLen, qMax(StateInfo<PackType::Data>::packMaxLen, StateInfo<PackType::Finish>::packLen)));

/* 协议全部使用的是大端 */
template<typename T>
static void localToBigEndian(void* pos, T value)
{
    /* 3文件长度 对方接收是大端 */
    value = qToBigEndian<T>(value);
    std::memcpy(pos, &value, sizeof(T));
}
template<typename T>
static T bigToLocalEndian(const void* pos)
{
    /* 3文件长度 对方接收是大端 */
    T tempValue{};
    std::memcpy(&tempValue, pos, sizeof(T));
    return qFromBigEndian<T>(tempValue);
}
/*
 * 请求包
 * 格式
 * 1字节类型 PackType
 * 64字节文件名
 * 4字节文件长度
 */
bool REQPack::toByteArray(QByteArray& array, QString& errMsg) const
{
    static_assert(sizeof(fileLen) == Def::fileContentLen, "协议中文件内容长度是4个字节");
    constexpr auto packLen = StateInfo<type>::packLen;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(type);
    pos += Def::typeIDLen;

    /* 2文件名长度 */
    if (fileName.isEmpty())
    {
        errMsg = "文件名不能为空";
        return false;
    }
    /* 协议中文件名长度固定为64字节，所以长度大于64字节的文件名将被截断 */
    auto byteName = fileName.toUtf8();
    auto size = byteName.size();
    size = size > Def::fileNameLen ? Def::fileNameLen : size;
    std::memcpy(&(buffer[pos]), byteName.data(), size);
    pos += Def::fileNameLen;

    /* 3文件长度 对方接收是大端 */
    localToBigEndian<decltype(fileLen)>(&(buffer[pos]), fileLen);

    array = QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
    return true;
}
bool REQPack::fromByteArray(const QByteArray& array, QString& errMsg)
{
    constexpr auto packLen = StateInfo<type>::packLen;
    if (array.size() < packLen)
    {
        errMsg = QString("数据包长度与协议不符合,发送请求协议长度:1+64+4,当前长度:%1").arg(array.size());
        return false;
    }
    auto data = array.data();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = data[pos];
    if (tempType != static_cast<char>(type))
    {
        errMsg = "数据包发送请求协议不符";
        return false;
    }
    pos += Def::typeIDLen;

    /* 2文件名 */
    qint32 index = pos + Def::fileNameLen - 1;
    for (; index >= pos; --index)
    {
        if (data[index] != '\0')
        {
            break;
        }
    }
    // 这样的话当刚好Def::typeIDLen长度的时候可能会出问题
    // fileName = QString::fromUtf8(&(data[pos]), Def::fileNameLen);
    fileName = QString::fromUtf8(&(data[pos]), index - Def::typeIDLen + 1);
    if (fileName.isEmpty())
    {
        errMsg = "数据包解析失败,文件名为空";
        return false;
    }
    pos += Def::fileNameLen;

    /* 3文件内容长度 */
    fileLen = bigToLocalEndian<decltype(fileLen)>(&(data[pos]));
    return true;
}

bool AnserPack::toByteArray(QByteArray& array, QString& errMsg) const
{
    static_assert(sizeof(fileID) == Def::fileIDLen, "协议中文件标识是4个字节");
    static_assert(sizeof(dataUnitNum) == Def::dataUnitIDLen, "协议中数据单元号是4个字节");
    constexpr auto packLen = StateInfo<type>::packLen;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(type);
    pos += Def::typeIDLen;

    /* 2文件名长度 */
    if (fileName.isEmpty())
    {
        errMsg = "文件名不能为空";
        return false;
    }
    /* 协议中文件名长度固定为64字节，所以长度大于64字节的文件名将被截断 */
    auto byteName = fileName.toUtf8();
    auto size = byteName.size();
    size = size > Def::fileNameLen ? Def::fileNameLen : size;
    std::memcpy(&(buffer[pos]), byteName.data(), size);
    pos += Def::fileNameLen;

    /* 3数据单元号 */
    localToBigEndian<decltype(dataUnitNum)>(&(buffer[pos]), dataUnitNum);
    pos += Def::dataUnitIDLen;

    /* 4文件标识 */
    localToBigEndian<decltype(fileID)>(&(buffer[pos]), fileID);

    array = QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
    return true;
}
bool AnserPack::fromByteArray(const QByteArray& array, QString& errMsg)
{
    constexpr auto packLen = StateInfo<type>::packLen;
    if (array.size() < packLen)
    {
        errMsg = QString("数据包长度与协议不符合,请求应答协议长度:1+64+4+2,当前长度:%1").arg(array.size());
        return false;
    }
    auto data = array.data();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = data[pos];
    if (tempType != static_cast<char>(type))
    {
        errMsg = "数据包请求应答协议不符";
        return false;
    }
    pos += Def::typeIDLen;

    /* 2文件名 */
    fileName = QString::fromUtf8(&(data[pos]), Def::fileNameLen);
    if (fileName.isEmpty())
    {
        errMsg = "数据包解析失败,文件名为空";
        return false;
    }
    pos += Def::fileNameLen;

    /* 3数据单元号 */
    dataUnitNum = bigToLocalEndian<decltype(dataUnitNum)>(&(data[pos]));
    pos += Def::dataUnitIDLen;

    /* 4文件标识 */
    fileID = bigToLocalEndian<decltype(fileID)>(&(data[pos]));
    return true;
}

bool FinishPack::toByteArray(QByteArray& array, QString& /*errMsg*/) const
{
    static_assert(sizeof(fileID) == Def::fileIDLen, "协议中文件标识是4个字节");
    constexpr auto packLen = StateInfo<type>::packLen;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(type);
    pos += Def::typeIDLen;

    /* 2文件标识 */
    localToBigEndian<decltype(fileID)>(&(buffer[pos]), fileID);

    array = QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
    return true;
}
bool FinishPack::fromByteArray(const QByteArray& array, QString& errMsg)
{
    constexpr auto packLen = StateInfo<type>::packLen;
    if (array.size() < packLen)
    {
        errMsg = QString("数据包长度与协议不符合,结束确认协议长度:1+2,当前长度:%1").arg(array.size());
        return false;
    }
    auto data = array.data();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = data[pos];
    if (tempType != static_cast<char>(type))
    {
        errMsg = "数据包请求应答协议不符";
        return false;
    }
    pos += Def::typeIDLen;

    /* 2文件标识 */
    fileID = bigToLocalEndian<decltype(fileID)>(&(data[pos]));
    return true;
}

bool DataPack::toByteArray(QByteArray& array, QString& /*errMsg*/) const
{
    static_assert(sizeof(fileID) == Def::fileIDLen, "协议中文件标识是4个字节");
    static_assert(sizeof(dataUnitNum) == Def::dataUnitIDLen, "协议中数据单元号是4个字节");
    constexpr auto packLen = StateInfo<type>::packMinLen;
    quint8 buffer[packLen]{};
    /* 清空buffer */
    std::memset(&buffer, '\0', static_cast<size_t>(packLen));
    int pos = 0; /* 当前指针的位置 */

    /* 1包类型 */
    buffer[pos] = static_cast<quint8>(type);
    pos += Def::typeIDLen;

    /* 2数据单元号 */
    localToBigEndian<decltype(dataUnitNum)>(&(buffer[pos]), dataUnitNum);
    pos += Def::dataUnitIDLen;

    /* 3文件标识 */
    localToBigEndian<decltype(fileID)>(&(buffer[pos]), fileID);
    pos += Def::fileIDLen;

    /* 4数据 */
    Q_ASSERT(data.size() <= Def::dataMaxLen);

    array = QByteArray(reinterpret_cast<const char*>(&(buffer[0])), packLen);
    array.append(data);
    return true;
}
int DataPack::fromByteArray(const QByteArray& array, const RecvContent& content, quint32& readLen, QString& errMsg)
{
    constexpr auto packLen = StateInfo<type>::packMinLen;
    auto arraySize = array.size();
    if (arraySize < packLen)
    {
        errMsg = QString("数据包长度与协议不符合,数据包最小协议长度:1+4+2+0,当前长度:%1").arg(array.size());
        return -1;
    }
    auto arrayData = array.data();
    /* 1包类型 */
    int pos = 0; /* 当前指针的位置 */
    auto tempType = arrayData[pos];
    if (tempType != static_cast<char>(type))
    {
        errMsg = "数据包请求应答协议不符";
        return -1;
    }
    pos += Def::typeIDLen;

    /* 2数据单元号 */
    dataUnitNum = bigToLocalEndian<decltype(dataUnitNum)>(&(arrayData[pos]));
    pos += Def::dataUnitIDLen;

    /* 3文件标识 */
    fileID = bigToLocalEndian<decltype(fileID)>(&(arrayData[pos]));
    pos += Def::fileIDLen;

    /* 检查数据长度 */
    /* 已经接收的数据长度 */
    auto recvLen = content.recvFileSize;
    /* 文件总长 */
    auto fileLength = content.curFileInfo.fileLength;
    /* 当前应该接收的数据包大小 */
    auto curRecvDataLen = fileLength > recvLen + Def::dataMaxLen ? Def::dataMaxLen : fileLength - recvLen;
    /* 数据长度不够,那等会再接收 */
    if (static_cast<quint32>(arraySize) - packLen < curRecvDataLen)
    {
        return 0;
    }

    /* 4数据 */
    data = QByteArray(&(arrayData[pos]), curRecvDataLen);
    readLen = static_cast<quint32>(packLen) + curRecvDataLen;
    return 1;
}
