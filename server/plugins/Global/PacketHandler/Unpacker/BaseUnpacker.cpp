#include "BaseUnpacker.h"

#include "DevProtocolSerialize.h"
#include "ParameterHelper.h"
#include "ProtocolXmlTypeHelper.h"

BaseUnpacker::BaseUnpacker(QObject* parent)
    : QObject(parent)
{
}

BaseUnpacker::~BaseUnpacker() {}

bool BaseUnpacker::unpack(const QByteArray& data, UnpackMessage& message)
{
    message.rawData = data;
    auto bak = data;

    bak >> message.header;
    if (!message.header.isValid())
    {
        return false;
    }

    // 判断协议头数据域长度和实际数据域长度是否相等
    if (message.header.dataLen != data.length() - DevProtocolLength::value)
    {
        return false;
    }

    return unpackBody(data.mid(DevProtocolLength::value), message);
}

bool BaseUnpacker::unpackParams(const QList<ParameterAttribute>& params, const QByteArray& paramBytes, QMap<QString, QVariant>& paramVals, int& index)
{
    ParameterAttribute param;

    for (int paramIndex = 0; paramIndex < params.size(); ++paramIndex)
    {
        param = params.at(paramIndex);

        QVariant paramDisplayVal;

        if (param.attributeType != AttributeType::AttributeType_Bit)
        {
            auto bytes = paramBytes.mid(index, param.length);
            if (!ParameterHelper::fromByteArray(param, bytes, paramDisplayVal))
            {
                qDebug() << "参数解析错误：" << param.desc << param.id << bytes.toHex() << paramDisplayVal;
                return false;
            }
            paramVals.insert(param.id, paramDisplayVal);
            index += param.length;
        }
        else
        {
            if (!unpackBitParams(params, paramVals, paramIndex, paramBytes, index))
            {
                return false;
            }
        }
    }

    return true;
}

bool BaseUnpacker::unpackParam(const ParameterAttribute& param, const QByteArray& paramBytes, QVariant& paramVal, int& index)
{
    if (param.attributeType != AttributeType::AttributeType_Bit)
    {
        auto bytes = paramBytes.mid(index, param.length);
        if (!ParameterHelper::fromByteArray(param, bytes, paramVal))
        {
            qDebug() << "参数解析错误：" << param.desc << param.id << bytes.toHex() << paramVal;
            return false;
        }
        index += param.length;

        return true;
    }

    return false;
}

bool BaseUnpacker::unpackBitParams(const QList<ParameterAttribute>& params, QMap<QString, QVariant>& paramVals, int& paramIndex,
                                   const QByteArray& paramBytes, int& index) const
{
    // 通过传输类型获取bit长度
    auto transferType = params.at(paramIndex).transferType;
    auto transferByteLen = ProtocolXmlTypeHelper::getAttributeTypeByteLength(transferType);
    auto transferBitLen = transferByteLen * 8;

    if (paramBytes.size() <= index + transferByteLen)
    {
        return false;
    }

    auto bytes = paramBytes.mid(index, transferByteLen);  // 取出该类型的字节数据
    index += transferByteLen;

    QList<ParameterAttribute> tempParams;  // 翻转一下
    int bitsLen = 0;
    for (paramIndex; paramIndex < params.size(); ++paramIndex)
    {
        auto param = params.at(paramIndex);
        bitsLen += param.length;

        // 大于了表示配置错误
        if (bitsLen > transferBitLen)
        {
            return false;
        }

        tempParams.append(param);
        if (bitsLen == transferBitLen)
        {
            break;
        }
    }

    QVariant transferValueVar;
    if (!ParameterHelper::fromByteArray(bytes, transferType, transferValueVar))
    {
        return false;
    }

    quint64 transferValue = transferValueVar.toULongLong();
    for (auto param : tempParams)
    {
        auto tempValue = transferValue;
        tempValue = tempValue << (sizeof(transferValue) * 8 - param.length);
        tempValue = tempValue >> (sizeof(transferValue) * 8 - param.length);

        paramVals[param.id] = tempValue;
    }

    return true;
}
