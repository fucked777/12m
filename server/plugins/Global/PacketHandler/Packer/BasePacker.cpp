#include "BasePacker.h"

#include "CConverter.h"
#include "DevProtocolSerialize.h"
#include "ParameterHelper.h"
#include "ProtocolXmlTypeHelper.h"

BasePacker::BasePacker(QObject* parent)
    : QObject(parent)
{
}
BasePacker::~BasePacker() {}

bool BasePacker::pack(const PackMessage& message, QByteArray& data, QString& errorMsg) const
{
    // 组数据域
    QByteArray bodyData;
    if (!packBody(message, bodyData, errorMsg))
    {
        return false;
    }

    // 组协议头
    QByteArray headerData;
    auto header = message.header;
    if (!header.date.isValid())
    {
        header.date = GlobalData::currentDate();
    }
    if (!header.time.isValid())
    {
        header.time = GlobalData::currentTime();
    }

    header.dataLen = static_cast<decltype(header.dataLen)>(bodyData.size());
    headerData << header;

    data.clear();
    data.append(headerData);
    data.append(bodyData);

    qDebug() << "包头：" << headerData.length() << headerData.toHex().toUpper();
    qDebug() << "包体：" << bodyData.length() << bodyData.toHex().toUpper();

    return true;
}

bool BasePacker::packParams(const QList<ParameterAttribute>& params, const QMap<QString, QVariant>& paramValMap, QByteArray& paramBytes) const
{
    paramBytes.resize(0);

    QByteArray bytes;
    ParameterAttribute param;
    for (int i = 0; i < params.size(); ++i)
    {
        param = params.at(i);
        bytes.resize(0);

        QVariant paramDisplayVal;
        if (param.fixValue.isValid())
        {
            paramDisplayVal = param.fixValue;
        }
        else
        {
            paramDisplayVal = paramValMap.value(param.id);
            if (!paramDisplayVal.isValid())
            {
                paramDisplayVal = param.initValue;
            }
        }

        // 不为 bit 类型
        if (param.attributeType != AttributeType_Bit)
        {
            if (!ParameterHelper::toByteArray(param, paramDisplayVal, bytes))
            {
                return false;
            }
        }
        else
        {
            if (!packBitParams(params, paramValMap, i, bytes))
            {
                return false;
            }
        }
        qWarning() << param.desc << param.id << paramDisplayVal << bytes.toHex();
        paramBytes.append(bytes);
    }

    return true;
}

bool BasePacker::packBitParams(const QList<ParameterAttribute>& params, const QMap<QString, QVariant>& paramVals, int& index,
                               QByteArray& paramBytes) const
{
    // 通过传输类型获取bit长度
    auto transferType = params.at(index).transferType;
    auto transferByteLen = ProtocolXmlTypeHelper::getAttributeTypeByteLength(transferType);
    auto transferBitLen = transferByteLen * 8;

    quint64 transferValue = 0;  // 实际传输的值
    int currentBitLen = 0;      // 当前bit长度
    for (index; index < params.size(); ++index)
    {
        auto param = params.at(index);
        if (param.attributeType != AttributeType::AttributeType_Bit)
        {
            return false;
        }

        currentBitLen += param.length;
        // 大于了表示配置错误
        if (currentBitLen > transferBitLen)
        {
            return false;
        }

        transferValue = transferValue << param.length;
        transferValue |= paramVals.value(param.id).toULongLong();

        // 当前bit长度和传输长度相等,表示按位参数组装完成
        if (currentBitLen == transferBitLen)
        {
            break;
        }
    }

    // 转为bytes
    QByteArray bytes;
    if (!ParameterHelper::toByteArray(transferType, transferValue, bytes))
    {
        return false;
    }
    paramBytes.append(bytes);

    return true;
}
