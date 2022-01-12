#include "ParameterHelper.h"

#include <QDate>
#include <QTime>
#include <math.h>

#include "CConverter.h"
#include "GlobalDefine.h"

bool ParameterHelper::toByteArray(const ParameterAttribute& param, const QVariant& paramDisplayVal, QByteArray& paramBytes)
{
    QVariant tempParamValue = 0;
    switch (param.displayFormat)
    {
    case DisplayFormat_Int8:
    case DisplayFormat_UInt8:
    case DisplayFormat_Int16:
    case DisplayFormat_UInt16:
    case DisplayFormat_Int32:
    case DisplayFormat_UInt32:
    case DisplayFormat_Int64:
    case DisplayFormat_UInt64:
    case DisplayFormat_Float:
    case DisplayFormat_Double:
    case DisplayFormat_Enum:
    case DisplayFormat_WdBm:
    case DisplayFormat_String: tempParamValue = paramDisplayVal; break;
    case DisplayFormat_Hex: tempParamValue = paramDisplayVal.toString().toULongLong(nullptr, 16); break;
    case DisplayFormat_HexString:
    {
        QByteArray bytes(param.length, 0x00);

        auto displayText = paramDisplayVal.toString();
        if (displayText.length() % 2 != 0)
        {
            displayText.prepend("0");
        }

        QString tempText;
        for (int i = 0; i < displayText.length(); i += 2)
        {
            tempText.prepend(displayText.mid(i, 2));
        }

        auto tempByte = QByteArray::fromHex(tempText.toStdString().c_str());
        bytes.replace(0, tempByte.length(), tempByte);

        tempParamValue = bytes;
    }
    break;
    case DisplayFormat_Bit: break;
    case DisplayFormat_IP:
    {
        auto hexIp = IP2Hex(paramDisplayVal.toString());
        tempParamValue = hexIp.toUInt(nullptr, 16);
    }
    break;
    case DisplayFormat_StrIP: tempParamValue = paramDisplayVal; break;
    case DisplayFormat_AccumDate:
    {
        auto accumBaseDate = QDate(2000, 1, 1);
        auto paramDate = QDate::fromString(paramDisplayVal.toString(), DATE_DISPLAY_FORMAT);
        if (!paramDate.isValid())
            return false;
        tempParamValue = accumBaseDate.daysTo(paramDate) + 1;
    }
    break;
    case DisplayFormat_BCDDate:
    {
        auto date = QDate::fromString(paramDisplayVal.toString(), DATE_DISPLAY_FORMAT);
        if (!date.isValid())
            return false;
        tempParamValue = date.toString(DATE_ORIGIN_FORMAT).toULongLong(nullptr, 16);
    }
    break;
    case DisplayFormat_BCDTime:
    {
        auto time = QTime::fromString(paramDisplayVal.toString(), TIME_DISPLAY_FORMAT);
        if (!time.isValid())
            return false;
        tempParamValue = time.msecsSinceStartOfDay();
    }
    break;
    case DisplayFormat_StrDate:
    {
        auto date = QDate::fromString(paramDisplayVal.toString(), DATE_DISPLAY_FORMAT);
        if (!date.isValid())
            return false;
        tempParamValue = date.toString(DATE_ORIGIN_FORMAT);
    }
    break;
    case DisplayFormat_StrTime:
    {
        auto time = QTime::fromString(paramDisplayVal.toString(), TIME_DISPLAY_FORMAT);
        if (!time.isValid())
            return false;
        tempParamValue = time.toString(TIME_ORIGIN_FORMAT2);
    }
    break;
    case DisplayFormat_StrDateTime:
    {
        auto datetime = QDateTime::fromString(paramDisplayVal.toString(), DATETIME_DISPLAY_FORMAT3);
        if (!datetime.isValid())
            return false;
        tempParamValue = datetime.toString(DATETIME_ORIGIN_FORMAT3);
    }
    break;
    case DisplayFormat_TracksNumber:
    {
        tempParamValue = handleTracksNumber(paramDisplayVal.toDouble(), param.formula, false);
    }
    break;
    case DisplayFormat_Invalid: return false;
    default: return false;
    }

    // char类型的固定字节长度  DisplayFormat_HexString这个显示类型不能转GBK，会丢失部分数据
    if (param.attributeType == AttributeType_Char && param.displayFormat != DisplayFormat_HexString)
    {
        QByteArray bytes(param.length, 0x00);
        auto tempByte = tempParamValue.toByteArray();

        // 默认全部用GBK字节发送，因为联试应答机终端是用的GBK，避免中文乱码
        auto gbk = QTextCodec::codecForName("GBK");
        auto utf8 = QTextCodec::codecForName("UTF-8");
        tempByte = gbk->fromUnicode(utf8->toUnicode(tempByte));

        // 去掉多于字节
        if (tempByte.length() > param.length)
        {
            tempByte.remove(param.length, tempByte.length() - param.length);
        }
        bytes.replace(0, tempByte.length(), tempByte);

        tempParamValue = bytes;
    }

    // 处理步进
    QVariant scaledParamVal = tempParamValue;
    if (!param.step.isEmpty() && param.step != "1" && tempParamValue.canConvert<double>())
    {
        scaledParamVal = handleStep(tempParamValue.toDouble(), param.step, false);
    }

    // 转为字节
    if (!toByteArray(param.attributeType, scaledParamVal, paramBytes))
    {
        return false;
    }

    return true;
}

bool ParameterHelper::toByteArray(AttributeType attrType, const QVariant& paramValue, QByteArray& paramBytes)
{
    QByteArray bytes;
    bytes.resize(0);

    switch (attrType)
    {
    case AttributeType_Char: bytes = paramValue.toByteArray(); break;
    case AttributeType_Int8: bytes = CConverter::toByteArray<qint8>(paramValue.toInt()); break;
    case AttributeType_UInt8: bytes = CConverter::toByteArray<quint8>(paramValue.toUInt()); break;
    case AttributeType_Short: bytes = CConverter::toByteArray<qint16>(paramValue.toInt()); break;
    case AttributeType_UShort: bytes = CConverter::toByteArray<quint16>(paramValue.toUInt()); break;
    case AttributeType_Int: bytes = CConverter::toByteArray<qint32>(paramValue.toInt()); break;
    case AttributeType_UInt: bytes = CConverter::toByteArray<quint32>(paramValue.toUInt()); break;
    case AttributeType_Long: bytes = CConverter::toByteArray<qint64>(paramValue.toLongLong()); break;
    case AttributeType_ULong: bytes = CConverter::toByteArray<quint64>(paramValue.toULongLong()); break;
    case AttributeType_Bit: break;
    case AttributeType_Invalid: return false;
    default: return false;
    }

    paramBytes.resize(0);
    paramBytes.append(bytes);

    return true;
}

bool ParameterHelper::fromByteArray(const ParameterAttribute& param, const QByteArray& paramBytes, QVariant& paramDisplayVal)
{
    QVariant paramVal;
    // 字节转为参数值
    if (!fromByteArray(paramBytes, param.attributeType, paramVal))
    {
        return false;
    }

    // 处理步进
    QVariant scaledParamVal = paramVal;
    if (!param.step.isEmpty() && param.step != "1")
    {
        scaledParamVal = handleStep(paramVal.toLongLong(), param.step);
    }

    // 参数值转为显示的值
    if (!fromByteArray(scaledParamVal, param, paramDisplayVal))
    {
        return false;
    }

    return true;
}

bool ParameterHelper::fromByteArray(const QVariant& paramVal, const ParameterAttribute& param, QVariant& paramDisplayVal)
{
    switch (param.displayFormat)
    {
    case DisplayFormat_Int8:
    case DisplayFormat_UInt8:
    case DisplayFormat_Int16:
    case DisplayFormat_UInt16:
    case DisplayFormat_Int32:
    case DisplayFormat_UInt32:
    case DisplayFormat_Int64:
    case DisplayFormat_UInt64:
    case DisplayFormat_Float:
    case DisplayFormat_Double:
    case DisplayFormat_WdBm:
    case DisplayFormat_LED: paramDisplayVal = paramVal; break;
    case DisplayFormat_Enum:
    {
        // 枚举值为整型，也有可能为字符串
        paramDisplayVal = getCurrentUnicode(paramVal.toByteArray());
    }
    break;
    case DisplayFormat_String:
    {
        paramDisplayVal = getCurrentUnicode(paramVal.toByteArray());
    }
    break;
    case DisplayFormat_Hex:
    {
        paramDisplayVal = QString::number(paramVal.toLongLong(), 16);
    }
    break;
    case DisplayFormat_HexString:
    {
        QString text;
        auto valueByteArr = paramVal.toByteArray();
        for (int i = 0; i < valueByteArr.size(); ++i)
        {
            text.prepend(valueByteArr.mid(i, 1).toHex().toUpper());
        }
        paramDisplayVal = text;
    }
    break;
    case DisplayFormat_Bit: break;
    case DisplayFormat_IP:
    {
        auto hexIp = QString::number(paramVal.toLongLong(), 16);
        paramDisplayVal = Hex2IP(hexIp);
    }
    break;
    case DisplayFormat_StrIP:
    {
        paramDisplayVal = getCurrentUnicode(paramVal.toByteArray());
    }
    break;
    case DisplayFormat_AccumDate:
    {
        auto accumBaseDate = QDate(2000, 1, 1);
        auto displayDate = accumBaseDate.addDays(paramVal.toULongLong() - 1);
        paramDisplayVal = displayDate.toString(DATE_DISPLAY_FORMAT);
    }
    break;
    case DisplayFormat_BCDDate:
    {
        auto date = QDate::fromString(QString::number(paramVal.toULongLong(), 16), DATE_ORIGIN_FORMAT);
        paramDisplayVal = date.toString(DATE_DISPLAY_FORMAT);
    }
    break;
    case DisplayFormat_BCDTime:
    {
        auto displayTime = QTime::fromMSecsSinceStartOfDay(paramVal.toInt());

        // BCD时间显示的时候精确到0.1ms，      QTimeEdit只能精确到1ms所以下发的时候精确的是1ms(有需要再改)
        paramDisplayVal = QString("%1%2").arg(displayTime.toString(TIME_DISPLAY_FORMAT)).arg((int)(paramVal.toDouble() * 10) % 10);
    }
    break;
    case DisplayFormat_StrDate:
    {
        paramDisplayVal = QDate::fromString(paramVal.toString(), DATE_ORIGIN_FORMAT).toString(DATE_DISPLAY_FORMAT);
    }
    break;
    case DisplayFormat_StrTime:
    {
        paramDisplayVal = QTime::fromString(paramVal.toString(), TIME_ORIGIN_FORMAT2).toString(TIME_DISPLAY_FORMAT);
    }
    break;
    case DisplayFormat_StrDateTime:
    {
        paramDisplayVal = QDateTime::fromString(paramVal.toString(), DATETIME_ORIGIN_FORMAT3).toString(DATETIME_DISPLAY_FORMAT3);
    }
    break;
    case DisplayFormat_TracksNumber:
    {
        paramDisplayVal = handleTracksNumber(paramVal.toDouble(), param.formula, true);
    }
    break;
    case DisplayFormat_Invalid: return false;
    default: return false;
    }

    return true;
}

bool ParameterHelper::fromByteArray(const QByteArray& paramBytes, AttributeType attrType, QVariant& paramVal)
{
    switch (attrType)
    {
    case AttributeType_Char: paramVal = paramBytes; break;
    case AttributeType_Int8: paramVal = CConverter::fromByteArray<typename std::decay<qint8>::type>(paramBytes); break;
    case AttributeType_UInt8: paramVal = CConverter::fromByteArray<typename std::decay<quint8>::type>(paramBytes); break;
    case AttributeType_Short: paramVal = CConverter::fromByteArray<typename std::decay<short>::type>(paramBytes); break;
    case AttributeType_UShort: paramVal = CConverter::fromByteArray<typename std::decay<ushort>::type>(paramBytes); break;
    case AttributeType_Int: paramVal = CConverter::fromByteArray<typename std::decay<int>::type>(paramBytes); break;
    case AttributeType_UInt: paramVal = CConverter::fromByteArray<typename std::decay<uint>::type>(paramBytes); break;
    case AttributeType_Long: paramVal = CConverter::fromByteArray<typename std::decay<qlonglong>::type>(paramBytes); break;
    case AttributeType_ULong: paramVal = CConverter::fromByteArray<typename std::decay<qulonglong>::type>(paramBytes); break;
    case AttributeType_Float: paramVal = CConverter::fromByteArray<typename std::decay<float>::type>(paramBytes); break;
    case AttributeType_Double: paramVal = CConverter::fromByteArray<typename std::decay<double>::type>(paramBytes); break;
    case AttributeType_Bit: break;
    case AttributeType_Invalid: return false;
    default: return false;
    }
    return true;
}
