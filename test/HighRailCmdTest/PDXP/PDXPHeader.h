#ifndef PDXPHEADER_H
#define PDXPHEADER_H

#include <QtCore>
#include <stdio.h>
#include <type_traits>

struct PDXPHeader
{
    quint8 version = 0;
    quint16 mid = 0;
    quint32 sid = 0;
    quint32 did = 0;
    quint32 bid = 0;
    quint32 no = 0;
    quint8 flag = 0;
    quint32 reserve = 0;
    quint16 date = 0;
    quint32 time = 0;
    quint16 len = 0;
};

using PDXPHeaderVersionLength = std::integral_constant<int, sizeof(PDXPHeader::version)>;
using PDXPHeaderMidLength = std::integral_constant<int, sizeof(PDXPHeader::mid)>;
using PDXPHeaderSidLength = std::integral_constant<int, sizeof(PDXPHeader::sid)>;
using PDXPHeaderDidLength = std::integral_constant<int, sizeof(PDXPHeader::did)>;
using PDXPHeaderBidLength = std::integral_constant<int, sizeof(PDXPHeader::bid)>;
using PDXPHeaderNoLength = std::integral_constant<int, sizeof(PDXPHeader::no)>;
using PDXPHeaderFlagLength = std::integral_constant<int, sizeof(PDXPHeader::flag)>;
using PDXPHeaderReserveLength = std::integral_constant<int, sizeof(PDXPHeader::reserve)>;
using PDXPHeaderDateLength = std::integral_constant<int, sizeof(PDXPHeader::date)>;
using PDXPHeaderTimeLength = std::integral_constant<int, sizeof(PDXPHeader::time)>;
using PDXPHeaderLenLength = std::integral_constant<int, sizeof(PDXPHeader::len)>;

using PDXPHeaderLength =                                         //
    std::integral_constant<int,                                  //
                           PDXPHeaderVersionLength::value +      //
                               PDXPHeaderMidLength ::value +     //
                               PDXPHeaderSidLength ::value +     //
                               PDXPHeaderDidLength ::value +     //
                               PDXPHeaderBidLength ::value +     //
                               PDXPHeaderNoLength ::value +      //
                               PDXPHeaderFlagLength ::value +    //
                               PDXPHeaderReserveLength::value +  //
                               PDXPHeaderDateLength ::value +    //
                               PDXPHeaderTimeLength ::value +    //
                               PDXPHeaderLenLength ::value>;

/* 此索引是从0开始的索引 */
using PDXPHeaderVersionIndex = std::integral_constant<int, 0>;
using PDXPHeaderMidIndex = std::integral_constant<int, PDXPHeaderVersionIndex::value + PDXPHeaderVersionLength::value>;
using PDXPHeaderSidIndex = std::integral_constant<int, PDXPHeaderMidIndex::value + PDXPHeaderMidLength ::value>;
using PDXPHeaderDidIndex = std::integral_constant<int, PDXPHeaderSidIndex::value + PDXPHeaderSidLength ::value>;
using PDXPHeaderBidIndex = std::integral_constant<int, PDXPHeaderDidIndex ::value + PDXPHeaderDidLength ::value>;
using PDXPHeaderNoIndex = std::integral_constant<int, PDXPHeaderBidIndex ::value + PDXPHeaderBidLength ::value>;
using PDXPHeaderFlagIndex = std::integral_constant<int, PDXPHeaderNoIndex ::value + PDXPHeaderNoLength ::value>;
using PDXPHeaderReserveIndex = std::integral_constant<int, PDXPHeaderFlagIndex ::value + PDXPHeaderFlagLength ::value>;
using PDXPHeaderDateIndex = std::integral_constant<int, PDXPHeaderReserveIndex::value + PDXPHeaderReserveLength::value>;
using PDXPHeaderTimeIndex = std::integral_constant<int, PDXPHeaderDateIndex ::value + PDXPHeaderDateLength ::value>;
using PDXPHeaderLenIndex = std::integral_constant<int, PDXPHeaderTimeIndex ::value + PDXPHeaderTimeLength ::value>;

#endif  // IPDXPHEADER_H
