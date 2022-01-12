#ifndef PARAMETERHELPER_H
#define PARAMETERHELPER_H

#include <QTextCodec>

#include "ProtocolXmlTypeDefine.h"

#include <QRegularExpression>

// 匹配小数步进
static bool matchDecimal(const QString& str)
{
    /* 20211106 wp?
     * QRegExp这个不支持多线程 会崩溃
     */
    QRegExp reg(R"(^0\.\d+$)");
    return reg.exactMatch(str);
}

template<class T>
static QVariant handleTracksNumber(const T& rawVal, const QString& text, bool multiplication = true)
{
    QRegularExpression reg;
    reg.setPattern(R"(pow\((.+?),\s{0,}(.+?)\))");

    auto match = reg.match(text);
    if (!match.isValid())
    {
        return 0;
    }

    bool isOk1, isOk2;
    auto val1 = match.captured(1).toDouble(&isOk1);
    auto val2 = match.captured(2).toDouble(&isOk2);

    if (!(isOk1 && isOk2))
    {
        return 0;
    }

    QVariant value;
    if (multiplication)
    {
        value = rawVal / pow(val1, val2);
    }
    else
    {
        value = rawVal * pow(val1, val2);
    }

    return value;
}

/* 处理缩放 rawVal: 需要缩放的值 stepStr:缩放大小  multiplication:乘或者除 */
template<class T>
static QVariant handleStep(const T& rawVal, const QString& scaleStr, bool multiplication = true)
{
    // 小数位数
    int decimalLen = 6;

    // 缩放值
    double scaleVal = 1;

    // 匹配小数步进
    if (matchDecimal(scaleStr))
    {
        scaleVal = scaleStr.toDouble();

        // 保留小数位数
        if (scaleStr.startsWith("0."))
        {
            decimalLen = scaleStr.length() - 2;
        }
    }
    // 匹配分数步进
    else if (scaleStr.indexOf("/"))
    {
        QString frontStep = scaleStr.section('/', 0, 0);
        QString endStep = scaleStr.section('/', 1, 1);
        if (endStep.toInt() >= 1 && frontStep.toInt() >= 1)
        {
            auto result = (rawVal * endStep.toInt()) / frontStep.toInt();
            return result;
        }
    }

    scaleVal = multiplication ? scaleVal : 1 / scaleVal;

    double result = rawVal * scaleVal;
    QString format = QString("%.%1f").arg(decimalLen);

    auto stdStr = format.toStdString();
    return QString::asprintf(stdStr.c_str(), result).toDouble();
}

static QString getCurrentUnicode(const QByteArray& bytes)
{
    QTextCodec::ConverterState state;
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(bytes.constData(), bytes.size(), &state);

    if (state.invalidChars > 0)
    {
        text = QTextCodec::codecForName("GBK")->toUnicode(bytes);
    }
    else
    {
        text = bytes;
    }
    return text;
}

static bool matchingIP(const QString& str)
{
    QRegExp reg("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?["
                "0-9][0-9]?)\\b");
    return reg.exactMatch(str);
}

static QString Hex2IP(const QString& hexIP)
{
    // 四字节，十六进制是8个字符，每两个表示一个ip段，每段不足补0
    if (hexIP.length() != 8)
        return "";

    QStringList ipSection;
    int i = 0;
    for (i = 0; i < 8; i += 2)
    {
        ipSection.prepend(QString::number(hexIP.mid(i, 2).toUInt(0, 16)));  // 目前只有DTE有IP上报的数据
    }
    auto ipStr = ipSection.join(".");

    return matchingIP(ipStr) ? ipStr : "";
}

static QString IP2Hex(const QString& ip)
{
    if (!matchingIP(ip))
        return "";

    QStringList ipSection = ip.split(".");
    QString hexIP;
    for (auto& section : ipSection)
    {
        hexIP.append(QString("%1").arg(QString(section).toUInt(), 2, 16, QChar('0')));
    }
    return hexIP;
}

class ParameterHelper
{
public:
    static bool toByteArray(const ParameterAttribute& param, const QVariant& paramDisplayVal, QByteArray& paramBytes);
    static bool toByteArray(AttributeType attrType, const QVariant& paramValue, QByteArray& paramBytes);
    static bool fromByteArray(const ParameterAttribute& param, const QByteArray& paramBytes, QVariant& paramDisplayVal);
    static bool fromByteArray(const QByteArray& paramBytes, AttributeType attrType, QVariant& paramVal);

private:
    static bool fromByteArray(const QVariant& paramVal, const ParameterAttribute& param, QVariant& paramDisplayVal);
};

#endif  // PARAMETERHELPER_H
