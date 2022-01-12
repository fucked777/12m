#include "PackPDXPHeader.h"
#include "CConverter.h"
#include <QDateTime>

QByteArray PackPDXPHeader::packHead(const PDXPHeader& header)
{
    QByteArray message;
    packVer(message, header.version);
    packMID(message, header.mid);
    packSIDAndDID(message, header.sid);
    packSIDAndDID(message, header.did);
    packBID(message, header.bid);
    packNo(message, header.no);
    packFlag(message);
    packReserve(message);
    packDate(message);
    packTime(message);

    return message;
}

Optional<PDXPHeader> PackPDXPHeader::unpackHead(const QByteArray& message)
{
    using ResType = Optional<PDXPHeader>;
    if (message.size() < PDXPHeaderLength::value)
    {
        return ResType(ErrorCode::InvalidArgument, "数据长度小于协议头长度无法解析");
    }

    QByteArray VERSION = message.mid(PDXPHeaderVersionIndex ::value, PDXPHeaderVersionLength ::value);
    QByteArray MID = message.mid(PDXPHeaderMidIndex ::value, PDXPHeaderMidLength ::value);
    QByteArray SID = message.mid(PDXPHeaderSidIndex ::value, PDXPHeaderSidLength ::value);
    QByteArray DID = message.mid(PDXPHeaderDidIndex ::value, PDXPHeaderDidLength ::value);
    QByteArray BID = message.mid(PDXPHeaderBidIndex ::value, PDXPHeaderBidLength ::value);
    QByteArray NO = message.mid(PDXPHeaderNoIndex ::value, PDXPHeaderNoLength ::value);
    QByteArray FLAG = message.mid(PDXPHeaderFlagIndex ::value, PDXPHeaderFlagLength ::value);
    QByteArray RESERVE = message.mid(PDXPHeaderReserveIndex ::value, PDXPHeaderReserveLength ::value);
    QByteArray DATE = message.mid(PDXPHeaderDateIndex ::value, PDXPHeaderDateLength ::value);
    QByteArray TIME = message.mid(PDXPHeaderTimeIndex ::value, PDXPHeaderTimeLength ::value);
    QByteArray LEN = message.mid(PDXPHeaderLenIndex ::value, PDXPHeaderLenLength ::value);

    PDXPHeader pdxpHeader;
    pdxpHeader.version = CConverter::fromByteArray<quint8>(VERSION);
    pdxpHeader.mid = CConverter::fromByteArray<quint16>(MID);
    pdxpHeader.sid = CConverter::fromByteArray<quint32>(SID);
    pdxpHeader.did = CConverter::fromByteArray<quint32>(DID);
    pdxpHeader.bid = CConverter::fromByteArray<quint32>(BID);
    pdxpHeader.no = CConverter::fromByteArray<quint32>(NO);
    pdxpHeader.flag = CConverter::fromByteArray<quint8>(FLAG);
    pdxpHeader.reserve = CConverter::fromByteArray<quint32>(RESERVE);
    pdxpHeader.date = CConverter::fromByteArray<quint16>(DATE);
    pdxpHeader.time = CConverter::fromByteArray<quint32>(TIME);
    pdxpHeader.len = CConverter::fromByteArray<quint16>(LEN);

    //判断协议内容长度是否正确
    if (pdxpHeader.len != message.size() - PDXPHeaderLength::value)
    {
        return ResType(ErrorCode::InvalidArgument, "协议长度与实际长度不符");
    }
    return ResType(pdxpHeader);
}

void PackPDXPHeader::packVer(QByteArray& message, short version)
{
    QByteArray type;
    type.resize(1);
    type[0] = version;
    message.append(type);
}

void PackPDXPHeader::packMID(QByteArray& message, ushort mid) { message.append(CConverter::toByteArray<ushort>(mid)); }

void PackPDXPHeader::packSIDAndDID(QByteArray& message, int address) { message.append(CConverter::toByteArray<qint32>(address)); }

void PackPDXPHeader::packBID(QByteArray& message, int messageType) { message.append(CConverter::toByteArray<qint32>(messageType)); }

void PackPDXPHeader::packNo(QByteArray& message, int index) { message.append(CConverter::toByteArray<qint32>(index)); }

void PackPDXPHeader::packFlag(QByteArray& message, int index)
{
    QByteArray data;
    data.resize(1);
    data[0] = index;
    message.append(data);
}

void PackPDXPHeader::packReserve(QByteArray& message)
{
    // qDebug()<<"Pack::packReserve";

    QByteArray reserve;
    reserve.resize(4);
    reserve[0] = 0x00;
    reserve[1] = 0x00;
    reserve[2] = 0x00;
    reserve[3] = 0x00;

    message.append(reserve);
}

void PackPDXPHeader::packDate(QByteArray& message)
{
    QDateTime currentDateTime = GlobalData::currentDateTime();

    QString defaultTimeStr = "2000-01-01 00:00:00";
    QDateTime defaultDateTime = QDateTime::fromString(defaultTimeStr, "yyyy-MM-dd hh:mm:ss");

    qint64 currentDateTimeNum = currentDateTime.toMSecsSinceEpoch() / 1000;
    qint64 defaultDateTimeNum = defaultDateTime.toMSecsSinceEpoch() / 1000;

    int ndaysec = 24 * 60 * 60;
    //当前相对于2000的天数
    quint16 days =
        (currentDateTimeNum - defaultDateTimeNum) / ndaysec + ((currentDateTimeNum - defaultDateTimeNum) % (ndaysec) + (ndaysec - 1)) / ndaysec - 1;

    QByteArray data;
    data.append(CConverter::toByteArray<ushort>(days));  // Time
    message.append(data);
}

void PackPDXPHeader::packTime(QByteArray& message)
{
    int time = QTime::currentTime().msecsSinceStartOfDay();

    QByteArray data;
    data.append(CConverter::toByteArray<qint32>(time));  // Time

    message.append(data);
}
