#include "DynamicConfigToWidgetUtility.h"
#include "Utility.h"
#include <QComboBox>
#include <QCursor>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSpinBox>
#include <QTextCursor>
#include <QTextEdit>
#include <QTreeWidget>
#include <QUuid>
#include <tuple>

DynamicConfigToWidgetType DynamicConfigToWidgetUtility::controlTypeFromStr(const QString& raw)
{
    auto type = raw.toUpper();
    if (type == "ENUM")
    {
        return DynamicConfigToWidgetType::Enum;
    }
    else if (type == "TEXTLINE")
    {
        return DynamicConfigToWidgetType::TextLine;
    }
    else if (type == "TEXTMULTIPLE")
    {
        return DynamicConfigToWidgetType::TextMultiple;
    }
    else if (type == "INTEGER")
    {
        return DynamicConfigToWidgetType::Integer;
    }
    else if (type == "DOUBLE")
    {
        return DynamicConfigToWidgetType::Double;
    }
    else if (type == "HEXNUM")
    {
        return DynamicConfigToWidgetType::HexNum;
    }
    else if (type == "HEXSTR")
    {
        return DynamicConfigToWidgetType::HexStr;
    }
    else if (type == "IP")
    {
        return DynamicConfigToWidgetType::IP;
    }

    return DynamicConfigToWidgetType::Unknown;
}

Optional<DynamicWidgetInfo> DynamicConfigToWidgetUtility::createWidget(const CommonToWidgetInfo& widgetInfo)
{
    using ResType = Optional<DynamicWidgetInfo>;
    switch (widgetInfo.type)
    {
    case DynamicConfigToWidgetType::Unknown:
    {
        return ResType(DynamicWidgetInfo(nullptr, widgetInfo));
    }
    case DynamicConfigToWidgetType::Enum:  // 下拉列表
    {
        /* 未注册 */
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<EnumToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<EnumToWidgetInfo>();
        auto temp = new QComboBox;

        for (auto iter = info.enumMap.begin(); iter != info.enumMap.end(); ++iter)
        {
            temp->addItem(iter.key(), iter.value());
        }
        info.initValue = info.initValue < 0 || info.initValue >= info.enumMap.size() ? 0 : info.initValue;
        temp->setCurrentIndex(info.initValue);
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::TextLine:  // 单行文本
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<TextLineToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<TextLineToWidgetInfo>();
        auto temp = new QLineEdit;
        if (info.length > 0)
        {
            temp->setMaxLength(info.length);
        }
        if (!info.regConstraints.isEmpty())
        {
            QRegExp regExp(info.regConstraints);
            temp->setValidator(new QRegExpValidator(regExp, temp));
        }
        temp->setPlaceholderText(info.placeholderText);
        temp->setText(info.initValue);
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::TextMultiple:  // 多行文本
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<TextMultipleToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<TextMultipleToWidgetInfo>();
        auto temp = new QTextEdit;

        temp->setPlaceholderText(info.placeholderText);
        temp->setPlainText(info.initValue);
        temp->setProperty("id", widgetInfo.id);
        if (info.length > 0)
        {
            temp->connect(temp, &QTextEdit::textChanged, [=]() {
                auto strText = temp->toPlainText();
                int length = strText.count();

                if (length > info.length)
                {
                    int position = temp->textCursor().position();

                    strText.remove(position - (length - info.length), length - info.length);

                    temp->setText(strText);

                    auto cursor = temp->textCursor();
                    cursor.setPosition(position - (length - info.length));
                    temp->setTextCursor(cursor);
                    // temp->setCursor(cursor);
                }
            });
        }
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::Integer:  // 整数
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<IntegerToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<IntegerToWidgetInfo>();
        auto temp = new QSpinBox;

        auto min = info.limitValid ? info.minValue : Limits::minValue<int>();
        temp->setMinimum(min);
        auto max = info.limitValid ? info.maxValue : Limits::maxValue<int>();
        temp->setMaximum(max);

        temp->setValue(info.initValue);
        temp->setToolTip(QString("参数范围: %1~%2").arg(min).arg(max));

        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::Double:  // 浮点
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<DoubleToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<DoubleToWidgetInfo>();
        auto temp = new QDoubleSpinBox;

        auto min = info.limitValid ? info.minValue : static_cast<double>(Limits::minValue<qint64>());
        temp->setMinimum(min);
        auto max = info.limitValid ? info.maxValue : static_cast<double>(Limits::maxValue<qint64>());
        temp->setMaximum(max);

        temp->setValue(info.initValue);
        if (info.decimals > 0)
        {
            temp->setDecimals(info.decimals);
        }

        // 去掉小数后多于的0
        QRegExp rx(R"((\.){0,1}0+$)");
        QString mintext = QString("%1").arg(min, 0, 'f').replace(rx, "");
        QString maxtext = QString("%1").arg(max, 0, 'f').replace(rx, "");
        temp->setToolTip(QString("参数范围: %1~%2").arg(mintext, maxtext));
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::HexNum:  // 16进制数字,获取到的值是数字
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<HexNumToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<HexNumToWidgetInfo>();
        auto temp = new QLineEdit;
        // [0-9a-fA-F]+
        // quint64 8个字节换算成字符串就是16个字符
        auto maxLen = static_cast<int>(sizeof(quint64) * 2);
        info.length = (info.length <= 0 || info.length > maxLen) ? maxLen : info.length;
        QRegExp regExp(Regexp::hex(info.length));
        temp->setValidator(new QRegExpValidator(regExp, temp));
        temp->setPlaceholderText(info.placeholderText);

        temp->setText(QString::number(info.initValue, 16));
        temp->setToolTip(QString("十六进制字符，且最多%1个字符").arg(info.length));
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::HexStr:  // 16进制文本,获取到的值是文本,与HexNum的区别是HexStr有可能转换为数字会溢出
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<HexStrToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<HexStrToWidgetInfo>();
        auto temp = new QLineEdit;
        QRegExp regExp(Regexp::hex(info.length));
        temp->setValidator(new QRegExpValidator(regExp, temp));
        temp->setPlaceholderText(info.placeholderText);

        QString toolTipText;
        if (info.length <= 0)
        {
            toolTipText = QString("十六进制字符");
        }
        else
        {
            toolTipText = QString("十六进制字符，且最多%1个字符").arg(info.length);
        }
        temp->setToolTip(toolTipText);

        if (!info.initValue.isEmpty())
        {
            temp->setText(info.initValue);
        }
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::IP:  // IP地址
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<IPToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<IPToWidgetInfo>();
        auto temp = new QLineEdit;
        temp->setValidator(new QRegExpValidator(QRegExp(Regexp::ip()), temp));
        if (!info.placeholderText.isEmpty())
        {
            temp->setPlaceholderText(info.placeholderText);
            temp->setToolTip(info.placeholderText);
        }
        if (!info.initValue.isEmpty())
        {
            temp->setText(info.initValue);
        }
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    case DynamicConfigToWidgetType::Label:
    {
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<LabelToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<LabelToWidgetInfo>();
        auto temp = new QLabel;
        temp->setText(info.initValue);
        temp->setProperty("id", widgetInfo.id);
        return ResType(DynamicWidgetInfo(temp, widgetInfo));
    }
    }
    return ResType(ErrorCode::InvalidArgument, "未识别的参数类型");
}

Optional<QVariant> DynamicConfigToWidgetUtility::getWidgetValue(const DynamicWidgetInfo& info)
{
    using ResType = Optional<QVariant>;
    if (info.widget == nullptr)
    {
        return ResType(ErrorCode::InvalidArgument, "当前widget为空");
    }

    switch (info.widgetInfo.type)
    {
    case DynamicConfigToWidgetType::Unknown:
    {
        break;
    }
    case DynamicConfigToWidgetType::Enum:
    {
        auto temp = qobject_cast<QComboBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        auto value = temp->currentData();
        if (info.widgetInfo.important && !value.isValid())
        {
            return ResType(ErrorCode::InvalidData, QString("%1无效").arg(info.widgetInfo.name));
        }
        return ResType(value);
    }
    case DynamicConfigToWidgetType::TextLine:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        auto text = temp->text();
        if (info.widgetInfo.important && text.isEmpty())
        {
            return ResType(ErrorCode::InvalidData, QString("%1为空").arg(info.widgetInfo.name));
        }
        return ResType(text);
    }
    case DynamicConfigToWidgetType::TextMultiple:
    {
        auto temp = qobject_cast<QTextEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        auto text = temp->toPlainText();
        if (info.widgetInfo.important && text.isEmpty())
        {
            return ResType(ErrorCode::InvalidData, QString("%1为空").arg(info.widgetInfo.name));
        }
        return ResType(text);
    }
    case DynamicConfigToWidgetType::Integer:
    {
        auto temp = qobject_cast<QSpinBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        return ResType(temp->value());
    }
    case DynamicConfigToWidgetType::Double:
    {
        auto temp = qobject_cast<QDoubleSpinBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        return ResType(temp->value());
    }
    case DynamicConfigToWidgetType::HexNum:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        auto tempValue = temp->text();
        bool isOk = false;
        auto value = tempValue.toULongLong(&isOk, 16);
        if (info.widgetInfo.important && !isOk && tempValue.isEmpty())
        {
            return ResType(ErrorCode::InvalidData, QString("%1无效").arg(info.widgetInfo.name));
        }
        return ResType(value);
    }
    case DynamicConfigToWidgetType::HexStr:
    {
        Q_ASSERT(info.widgetInfo.toWidgetInfo.canConvert<HexStrToWidgetInfo>());
        auto tooWidgetInfo = info.widgetInfo.toWidgetInfo.value<HexStrToWidgetInfo>();
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);

        auto value = temp->text();
        QRegularExpression ip(Regexp::hex(tooWidgetInfo.length));
        auto regularExpressionMatch = ip.match(value);
        if (info.widgetInfo.important && !regularExpressionMatch.hasMatch())
        {
            return ResType(ErrorCode::InvalidData, QString("%1无效").arg(info.widgetInfo.name));
        }
        return ResType(value);
    }
    case DynamicConfigToWidgetType::IP:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        auto value = temp->text();
        QRegularExpression ip(Regexp::ip());
        auto regularExpressionMatch = ip.match(value);
        if (info.widgetInfo.important && !regularExpressionMatch.hasMatch())
        {
            return ResType(ErrorCode::InvalidData, QString("%1无效").arg(info.widgetInfo.name));
        }
        return ResType(value);
    }
    case DynamicConfigToWidgetType::Label:
    {
        auto temp = qobject_cast<QLabel*>(info.widget);
        Q_ASSERT(temp != nullptr);
        return ResType(temp->text());
    }
    }

    return ResType(ErrorCode::InvalidArgument, "未识别的参数类型");
}
void DynamicConfigToWidgetUtility::setWidgetValue(const DynamicWidgetInfo& info, const QVariant& value)
{
    if (info.widget == nullptr || !value.isValid())
    {
        return;
    }
    switch (info.widgetInfo.type)
    {
    case DynamicConfigToWidgetType::Unknown:
    {
        break;
    }
    case DynamicConfigToWidgetType::Enum:
    {
        Q_ASSERT(info.widgetInfo.toWidgetInfo.canConvert<EnumToWidgetInfo>());
        auto toWidgetInfo = info.widgetInfo.toWidgetInfo.value<EnumToWidgetInfo>();

        auto temp = qobject_cast<QComboBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        for (auto iter = toWidgetInfo.enumMap.begin(); iter != toWidgetInfo.enumMap.end(); ++iter)
        {
            if (iter->toString() == value.toString())
            {
                temp->setCurrentText(iter.key());
                break;
            }
        }
        break;
    }
    case DynamicConfigToWidgetType::IP:
    case DynamicConfigToWidgetType::HexStr:
    case DynamicConfigToWidgetType::TextLine:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setText(value.toString());
        break;
    }
    case DynamicConfigToWidgetType::TextMultiple:
    {
        auto temp = qobject_cast<QTextEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setText(value.toString());
        break;
    }
    case DynamicConfigToWidgetType::Integer:
    {
        auto temp = qobject_cast<QSpinBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setValue(value.toInt());
        break;
    }
    case DynamicConfigToWidgetType::Double:
    {
        auto temp = qobject_cast<QDoubleSpinBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setValue(value.toDouble());
        break;
    }
    case DynamicConfigToWidgetType::HexNum:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setText(QString::number(value.toULongLong(), 16));
        break;
    }
    case DynamicConfigToWidgetType::Label:
    {
        auto temp = qobject_cast<QLabel*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setText(value.toString());
        break;
    }
    }
}
void DynamicConfigToWidgetUtility::resetValue(const DynamicWidgetInfo& info)
{
    if (info.widget == nullptr)
    {
        return;
    }

    switch (info.widgetInfo.type)
    {
    case DynamicConfigToWidgetType::Unknown:
    {
        break;
    }
    case DynamicConfigToWidgetType::Enum:
    {
        auto temp = qobject_cast<QComboBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setCurrentIndex(0);
        break;
    }
    case DynamicConfigToWidgetType::Integer:
    {
        auto temp = qobject_cast<QSpinBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setValue(0);
        break;
    }
    case DynamicConfigToWidgetType::Double:
    {
        auto temp = qobject_cast<QDoubleSpinBox*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setValue(0.0);
        break;
    }
    case DynamicConfigToWidgetType::IP:
    case DynamicConfigToWidgetType::HexStr:
    case DynamicConfigToWidgetType::TextLine:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->clear();
        break;
    }
    case DynamicConfigToWidgetType::HexNum:
    {
        auto temp = qobject_cast<QLineEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->setText("0");
        break;
    }
    case DynamicConfigToWidgetType::TextMultiple:
    {
        auto temp = qobject_cast<QTextEdit*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->clear();
        break;
    }
    case DynamicConfigToWidgetType::Label:
    {
        auto temp = qobject_cast<QLabel*>(info.widget);
        Q_ASSERT(temp != nullptr);
        temp->clear();
        break;
    }
    }
}

Optional<CommonToWidgetInfo> DynamicConfigToWidgetUtility::parseXmlItem(QDomElement& element)
{
    using ResType = Optional<CommonToWidgetInfo>;
    CommonToWidgetInfo commonToWidgetInfo;
    auto typeStr = element.attribute("type").trimmed();
    commonToWidgetInfo.type = DynamicConfigToWidgetUtility::controlTypeFromStr(typeStr);
    commonToWidgetInfo.id = element.attribute("id").trimmed();
    commonToWidgetInfo.name = element.attribute("name").trimmed();
    commonToWidgetInfo.desc = element.attribute("desc").trimmed();
    commonToWidgetInfo.important = QVariant(element.attribute("important", "true").trimmed()).toBool();
    commonToWidgetInfo.onlyOne = QVariant(element.attribute("onlyOne").trimmed()).toBool();

    switch (commonToWidgetInfo.type)
    {
    case DynamicConfigToWidgetType::Unknown:
    {
        break;
    }
    case DynamicConfigToWidgetType::Enum:
    {
        EnumToWidgetInfo info;
        info.initValue = element.attribute("default").toInt();
        auto enumValueList = element.attribute("enumValue").split("|", QString::SkipEmptyParts);
        for (auto& item : enumValueList)
        {
            auto kv = item.split("=", QString::SkipEmptyParts);
            /* 写错了 */
            if (kv.size() != 2)
            {
                return ResType(ErrorCode::InvalidData, QString("配置文件错误:%1").arg(item));
            }
            info.enumMap.insert(kv.at(0), kv.at(1));
        }
        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<EnumToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::TextLine:
    {
        TextLineToWidgetInfo info;
        info.initValue = element.attribute("default").trimmed();
        info.length = element.attribute("len").trimmed().toInt();
        info.placeholderText = element.attribute("placeholderText").trimmed();
        info.regConstraints = element.attribute("reg").trimmed();

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<TextLineToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::TextMultiple:
    {
        TextMultipleToWidgetInfo info;
        info.initValue = element.attribute("default").trimmed();
        info.placeholderText = element.attribute("placeholderText").trimmed();
        info.length = element.attribute("len").trimmed().toInt();

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<TextMultipleToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::Integer:
    {
        IntegerToWidgetInfo info;
        info.minValue = element.attribute("min").trimmed().toInt();
        info.maxValue = element.attribute("max").trimmed().toInt();
        info.initValue = element.attribute("default").trimmed().toInt();
        info.limitValid = element.hasAttribute("max");

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<IntegerToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::Double:
    {
        DoubleToWidgetInfo info;
        info.minValue = element.attribute("min").trimmed().toDouble();
        info.maxValue = element.attribute("max").trimmed().toDouble();
        info.initValue = element.attribute("default").trimmed().toDouble();
        info.limitValid = element.hasAttribute("max");
        info.decimals = element.attribute("decimals").trimmed().toInt();

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<DoubleToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::HexNum:
    {
        HexNumToWidgetInfo info;

        info.initValue = element.attribute("default").trimmed().toULongLong(nullptr, 16);
        info.length = element.attribute("len").trimmed().toInt();
        info.placeholderText = element.attribute("placeholderText").trimmed();

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<HexNumToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::HexStr:
    {
        HexStrToWidgetInfo info;

        info.initValue = element.attribute("default").trimmed();
        info.length = element.attribute("len").trimmed().toInt();
        info.placeholderText = element.attribute("placeholderText").trimmed();

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<HexStrToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::IP:
    {
        IPToWidgetInfo info;
        info.initValue = element.attribute("default").trimmed();
        info.placeholderText = element.attribute("placeholderText").trimmed();

        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<IPToWidgetInfo>(info);
        break;
    }
    case DynamicConfigToWidgetType::Label:
    {
        LabelToWidgetInfo info;
        info.initValue = element.attribute("default").trimmed();
        commonToWidgetInfo.toWidgetInfo = QVariant::fromValue<LabelToWidgetInfo>(info);
        break;
    }
    default: return ResType(ErrorCode::InvalidData, QString("错误的类型:%1").arg(typeStr));
    }
    return ResType(commonToWidgetInfo);
}
