#include "ProtocolXmlTypeHelper.h"

DisplayFormat ProtocolXmlTypeHelper::getDisplayFormat(const QString& str)
{
    if (str == NULL || str.isEmpty())
    {
        return DisplayFormat::DisplayFormat_Invalid;
    }

    QString text = str.toUpper();
    DisplayFormat displayFormat = DisplayFormat::DisplayFormat_Invalid;

    if ("ENUM" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Enum;
    }
    else if ("INT8" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Int8;
    }
    else if ("UINT8" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_UInt8;
    }
    else if ("INT16" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Int16;
    }
    else if ("UINT16" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_UInt16;
    }
    else if ("INT32" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Int32;
    }
    else if ("UINT32" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_UInt32;
    }
    else if ("FLOAT" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Float;
    }
    else if ("DOUBLE" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Double;
    }
    else if ("INT64" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Int64;
    }
    else if ("UINT64" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_UInt64;
    }
    else if ("STRING" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_String;
    }
    else if ("BIT" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Bit;
    }
    else if ("HEX" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_Hex;
    }
    else if ("HEXSTRING" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_HexString;
    }
    else if ("LED" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_LED;
    }
    else if ("ACCUMDATE" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_AccumDate;
    }
    else if ("BCDDATE" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_BCDDate;
    }
    else if ("BCDTIME" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_BCDTime;
    }
    else if ("STRDATE" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_StrDate;
    }
    else if ("STRTIME" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_StrTime;
    }
    else if ("STRDATETIME" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_StrDateTime;
    }
    else if ("IP" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_IP;
    }
    else if ("STRIP" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_StrIP;
    }
    else if ("CONVERTERWDBM" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_WdBm;
    }
    else if ("TRACKSNUMBER" == text)
    {
        displayFormat = DisplayFormat::DisplayFormat_TracksNumber;
    }
    else
    {
        displayFormat = DisplayFormat::DisplayFormat_Invalid;
    }
    return displayFormat;
}

int ProtocolXmlTypeHelper::getAttributeTypeByteLength(const QString& str)
{
    if (str.isEmpty())
    {
        return 0;
    }

    auto text = str.toUpper();
    if (text == "CHAR")
    {
        return sizeof(char);
    }
    else if (text == "INT8")
    {
        return sizeof(qint8);
    }
    else if (text == "UINT8")
    {
        return sizeof(quint8);
    }
    else if (text == "SHORT")
    {
        return sizeof(short);
    }
    else if (text == "USHORT")
    {
        return sizeof(ushort);
    }
    else if (text == "INT")
    {
        return sizeof(int);
    }
    else if (text == "UINT")
    {
        return sizeof(uint);
    }
    else if (text == "FLOAT")
    {
        return sizeof(float);
    }
    else if (text == "DOUBLE")
    {
        return sizeof(double);
    }
    else if (text == "LONG")
    {
        return sizeof(qint64);
    }
    else if (text == "ULONG")
    {
        return sizeof(quint64);
    }
    return 0;
}

int ProtocolXmlTypeHelper::getAttributeTypeByteLength(const AttributeType& type)
{
    switch (type)
    {
    case AttributeType_Char: return sizeof(char);
    case AttributeType_Int8:
    case AttributeType_UInt8: return sizeof(quint8);
    case AttributeType_Short:
    case AttributeType_UShort: return sizeof(quint16);
    case AttributeType_Int:
    case AttributeType_UInt: return sizeof(quint32);
    case AttributeType_Float: return sizeof(float);
    case AttributeType_Double: return sizeof(double);
    case AttributeType_Long:
    case AttributeType_ULong: return sizeof(quint64);
    }

    return 0;
}

AttributeType ProtocolXmlTypeHelper::getAttributeType(const QString& str)
{
    if (str.isEmpty())
    {
        return AttributeType::AttributeType_Invalid;
    }

    auto text = str.toUpper();
    AttributeType attrType = AttributeType::AttributeType_Invalid;
    if (text == "CHAR")
    {
        attrType = AttributeType::AttributeType_Char;
    }
    else if (text == "INT8")
    {
        attrType = AttributeType::AttributeType_Int8;
    }
    else if (text == "UINT8")
    {
        attrType = AttributeType::AttributeType_UInt8;
    }
    else if (text == "SHORT")
    {
        attrType = AttributeType::AttributeType_Short;
    }
    else if (text == "USHORT")
    {
        attrType = AttributeType::AttributeType_UShort;
    }
    else if (text == "INT")
    {
        attrType = AttributeType::AttributeType_Int;
    }
    else if (text == "UINT")
    {
        attrType = AttributeType::AttributeType_UInt;
    }
    else if (text == "FLOAT")
    {
        attrType = AttributeType::AttributeType_Float;
    }
    else if (text == "LONG")
    {
        attrType = AttributeType::AttributeType_Long;
    }
    else if (text == "ULONG")
    {
        attrType = AttributeType::AttributeType_ULong;
    }
    else if (text == "BIT")
    {
        attrType = AttributeType::AttributeType_Bit;
    }
    return attrType;
}
