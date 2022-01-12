#include "WNumber.h"

#include "ControlAttribute.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <QToolTip>
#include <QWidget>
#include <type_traits>

template<typename T>
class WNumberImpl
{
public:
    ParameterAttribute paraAttr;
    /* 当前value */
    T value{};
    QList<T> max{};
    QList<T> min{};
    /* 小数点后精度 */
    QString regDecimal;
    QString oldValue;
    QLineEdit* valueWidget{ nullptr };
    QLabel* unitLabel{ nullptr };
    QLabel* nameLabel{ nullptr };
    bool isValid{ false };
};
static bool matchingNum(const QString& str)
{
    /*
     * -0
     * 0
     * 001
     * -00123
     */
    QRegExp reg(R"(^-?[0-9]\d*$)");
    return reg.exactMatch(str);
}
static bool matchingDouble(const QString& str, const QString& regStr, QString& res)
{
    QRegularExpression rege(regStr);
    auto match = rege.match(str);
    if (!match.hasMatch())
    {
        return false;
    }
    res = match.captured(0);
    return !res.isEmpty();
}

static bool matchingDouble(const QString& str, const QString& regStr)
{
    QRegExp reg(regStr);
    return reg.exactMatch(str);
}

static int getDoubleDecimal(const QString& str)
{
    // 去掉末尾的0
    QString text1 = str;
    text1 = text1.replace(QRegExp(R"((\.){0,1}0+$)"), "");

    // 获取保留小数位数
    QString text2(R"((^\d+)\.([0-9]+))");
    QRegularExpression reg(text2);
    auto match = reg.match(text1);

    if (!match.hasMatch())
    {
        return 0;
    }
    return match.capturedLength(2);
}

template<typename T>
static bool checkValueRange(const QList<T>& min, const QList<T>& max, const T& value)
{
    auto size = min.size();
    Q_ASSERT(size == max.size());
    for (int i = 0; i < size; ++i)
    {
        auto& minItem = min.at(i);
        auto& maxItem = max.at(i);
        /* 验证范围 */
        if (((value > maxItem) || (value < minItem)))
        {
            continue;
        }
        return true;
    }
    return false;
}

WUNumber::WUNumber(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WNumberImpl<quint64> }
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    // m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->valueWidget->setCursorPosition(0);
    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 0, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    setNumRange("", "");
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WUNumber::textChanged);
    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WUNumber::editingFinished);
}

WUNumber::~WUNumber() { delete m_impl; }

void WUNumber::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
    paramAttr = attr;
}

void WUNumber::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->nameLabel->width();
    m_impl->nameLabel->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WUNumber::setValueWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->valueWidget->width();
    m_impl->valueWidget->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WUNumber::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WUNumber::resetValue() { setValue(m_impl->min.first()); }
void WUNumber::setValue(const QVariant& value)
{
    auto valueString = value.toString();
    if (valueString.isEmpty())
    {
        return;
    }

    m_impl->valueWidget->setText(valueString);
}

QVariant WUNumber::value() { return m_impl->isValid ? m_impl->value : m_impl->min.first(); }

QVariant WUNumber::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
QString WUNumber::text() { return m_impl->valueWidget->text().trimmed(); }

void WUNumber::setMinNum(const QVariant& minNum)
{
    if (m_impl->max.isEmpty())
    {
        setNumRange(minNum, QVariant(Limit::max<quint64>()));
        return;
    }
    QString maxStr;
    for (auto& item : m_impl->max)
    {
        maxStr += QString("%1|").arg(item);
    }
    maxStr.chop(1);
    setNumRange(minNum, QVariant(maxStr));
}

void WUNumber::setMaxNum(const QVariant& maxNum)
{
    if (m_impl->min.isEmpty())
    {
        setNumRange(QVariant(Limit::min<quint64>()), maxNum);
        return;
    }
    QString minStr;
    for (auto& item : m_impl->min)
    {
        minStr += QString("%1|").arg(item);
    }
    minStr.chop(1);
    setNumRange(QVariant(minStr), maxNum);
}

void WUNumber::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    m_impl->min.clear();
    m_impl->max.clear();
    auto minList = minNum.toString().split("|");
    auto maxList = maxNum.toString().split("|");
    auto size = minList.size() > maxList.size() ? maxList.size() : minList.size();
    if (size <= 0)
    {
        m_impl->min << getMin(m_impl->paraAttr.attributeType);
        m_impl->max << getMax(m_impl->paraAttr.attributeType);
    }
    else
    {
        bool convertMin = false;
        bool convertMax = false;
        for (int i = 0; i < size; ++i)
        {
            auto& minStr = minList.at(i);
            auto& maxStr = maxList.at(i);
            auto tempMin = minStr.toULongLong(&convertMin);
            auto tempMax = maxStr.toULongLong(&convertMax);
            if (!convertMax || !convertMin)
            {
                continue;
            }
            m_impl->min << tempMin;
            m_impl->max << tempMax;
        }
        if (m_impl->min.isEmpty())
        {
            m_impl->min << getMin(m_impl->paraAttr.attributeType);
            m_impl->max << getMax(m_impl->paraAttr.attributeType);
        }
    }
    /* 生成Tips */
    size = m_impl->min.size();
    QString tip;
    for (int i = 0; i < size; ++i)
    {
        tip += QString("%1~%2,").arg(m_impl->min.at(i)).arg(m_impl->max.at(i));
    }
    tip.chop(1);
    tip = QString("当前输入范围为%1").arg(tip);
    m_impl->valueWidget->setToolTip(tip);

    // 默认值为最小值
    setValue(m_impl->min.first());
    // 当修改的值与原值相等时也触发textChanged信号,更新超出范围的样式
    if (m_impl->min.first() == m_impl->value)
        emit m_impl->valueWidget->textChanged(m_impl->valueWidget->text());

    m_impl->value = m_impl->min.first();
    m_impl->oldValue = QString::number(m_impl->value);
    m_impl->isValid = true;
}

void WUNumber::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WUNumber::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WUNumber::setParamDesc(const QString& desc) { m_impl->nameLabel->setText(desc); }
void WUNumber::setUnit(const QString& unit)
{
    auto tempUnit = unit.trimmed();
    m_impl->unitLabel->setText(tempUnit);
    QFont font = m_impl->unitLabel->font();
    font.setPointSize(16);
    QFontMetrics fm(font);
    auto rect = fm.boundingRect(tempUnit);

    auto unitWidth = rect.width() + UnitExtendedWidth;
    m_impl->unitLabel->setFixedWidth(unitWidth);
    m_impl->valueWidget->setTextMargins(0, 0, unitWidth, 0);
}
void WUNumber::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
void WUNumber::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}
void WUNumber::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}

void WUNumber::textChanged(const QString& newValue)
{
    bool isConvert = false;
    auto newValueNum = newValue.toULongLong(&isConvert);
    if (!isConvert)
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    /* 验证范围 */
    if (!checkValueRange(m_impl->min, m_impl->max, newValueNum))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    m_impl->isValid = true;
    m_impl->value = newValueNum;
    m_impl->oldValue = QString::number(newValueNum);
    outOfRangesLineEdit(m_impl->valueWidget, false);
    emit sig_valueChanged(QVariant(newValue));
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}
/*********************************************************************************************************/
WINumber::WINumber(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WNumberImpl<qint64> }
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    //  m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->valueWidget->setCursorPosition(0);

    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 1, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    // this->setMinimumWidth(ControlWidget);
    this->setFixedWidth(ControlWidgetI);
    // this->setMaximumHeight(ControlHeight);

    setNumRange("", "");
    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WINumber::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WINumber::textChanged);
}

void WINumber::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->nameLabel->width();
    m_impl->nameLabel->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WINumber::setValueWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->valueWidget->width();
    m_impl->valueWidget->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WINumber::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
WINumber::~WINumber() { delete m_impl; }

void WINumber::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
    paramAttr = attr;
}

void WINumber::resetValue() { setValue(m_impl->min.first()); }
void WINumber::setValue(const QVariant& value)
{
    auto valueString = value.toString();

    if (valueString.isEmpty())
    {
        return;
    }
    m_impl->valueWidget->setText(valueString);
}
QString WINumber::text() { return m_impl->valueWidget->text().trimmed(); }

QVariant WINumber::value()
{
    // return ui->valueWidget->text().trimmed();
    return m_impl->isValid ? m_impl->value : m_impl->min.first();
}
QVariant WINumber::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
void WINumber::setMinNum(const QVariant& minNum)
{
    if (m_impl->max.isEmpty())
    {
        setNumRange(minNum, QVariant(Limit::max<qint64>()));
        return;
    }
    QString maxStr;
    for (auto& item : m_impl->max)
    {
        maxStr += QString("%1|").arg(item);
    }
    maxStr.chop(1);
    setNumRange(minNum, QVariant(maxStr));
}

void WINumber::setMaxNum(const QVariant& maxNum)
{
    if (m_impl->min.isEmpty())
    {
        setNumRange(QVariant(Limit::min<qint64>()), maxNum);
        return;
    }
    QString minStr;
    for (auto& item : m_impl->min)
    {
        minStr += QString("%1|").arg(item);
    }
    minStr.chop(1);
    setNumRange(QVariant(minStr), maxNum);
}

void WINumber::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    m_impl->min.clear();
    m_impl->max.clear();
    auto minList = minNum.toString().split("|");
    auto maxList = maxNum.toString().split("|");
    auto size = minList.size() > maxList.size() ? maxList.size() : minList.size();
    if (size <= 0)
    {
        m_impl->min << getMin(m_impl->paraAttr.attributeType);
        m_impl->max << getMax(m_impl->paraAttr.attributeType);
    }
    else
    {
        bool convertMin = false;
        bool convertMax = false;
        for (int i = 0; i < size; ++i)
        {
            auto& minStr = minList.at(i);
            auto& maxStr = maxList.at(i);
            auto tempMin = minStr.toLongLong(&convertMin);
            auto tempMax = maxStr.toLongLong(&convertMax);
            if (!convertMax || !convertMin)
            {
                continue;
            }
            m_impl->min << tempMin;
            m_impl->max << tempMax;
        }
        if (m_impl->min.isEmpty())
        {
            m_impl->min << getMin(m_impl->paraAttr.attributeType);
            m_impl->max << getMax(m_impl->paraAttr.attributeType);
        }
    }
    /* 生成Tips */
    size = m_impl->min.size();
    QString tip;
    for (int i = 0; i < size; ++i)
    {
        tip += QString("%1~%2,").arg(m_impl->min.at(i)).arg(m_impl->max.at(i));
    }
    tip.chop(1);
    tip = QString("当前输入范围为%1").arg(tip);
    m_impl->valueWidget->setToolTip(tip);

    // 默认值为最小值
    setValue(m_impl->min.first());
    // 当修改的值与原值相等时也触发textChanged信号,更新超出范围的样式
    if (m_impl->min.first() == m_impl->value)
        emit m_impl->valueWidget->textChanged(m_impl->valueWidget->text());

    m_impl->value = m_impl->min.first();
    m_impl->oldValue = QString::number(m_impl->value);
    m_impl->isValid = true;
}

void WINumber::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WINumber::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WINumber::setParamDesc(const QString& desc) { m_impl->nameLabel->setText(desc); }
void WINumber::setUnit(const QString& unit)
{
    auto tempUnit = unit.trimmed();
    m_impl->unitLabel->setText(tempUnit);

    QFont font = m_impl->unitLabel->font();
    font.setPointSize(16);
    QFontMetrics fm(font);
    auto rect = fm.boundingRect(tempUnit);

    auto unitWidth = rect.width() + UnitExtendedWidth;
    m_impl->unitLabel->setFixedWidth(unitWidth);
    m_impl->valueWidget->setTextMargins(0, 0, unitWidth, 0);
}
void WINumber::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
void WINumber::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}
void WINumber::textChanged(const QString& newValue)
{
    bool isConvert = false;
    auto newValueNum = newValue.toLongLong(&isConvert);
    if (!isConvert)
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    /* 验证范围 */
    if (!checkValueRange(m_impl->min, m_impl->max, newValueNum))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    //这个只针对跟踪基带命令里的偏移量参数，这个参数不能有0值
    if (this->objectName() == QString("Offset"))
    {
        if (newValueNum == 0)
        {
            m_impl->isValid = false;
            outOfRangesLineEdit(m_impl->valueWidget, true);
            return;
        }
    }

    m_impl->oldValue = QString::number(newValueNum);
    m_impl->value = newValueNum;
    m_impl->isValid = true;
    outOfRangesLineEdit(m_impl->valueWidget, false);
}
void WINumber::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
    /*
     * 0123  像这样的字符串这里也算合法的
     * 当输入完成后给改为---->123
     */
}

/*************************************************************************************************************************/
WDouble::WDouble(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WNumberImpl<double> }
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    //  m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->valueWidget->setCursorPosition(0);

    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 5, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    // this->setMinimumWidth(ControlWidget);
    this->setFixedWidth(ControlWidgetI);
    // this->setMaximumHeight(ControlHeight);

    /* 默认精度为6位小数 */

    setDecimal(6);
    setNumRange("", "");

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WDouble::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WDouble::textChanged);
}

WDouble::~WDouble() { delete m_impl; }

void WDouble::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
    setDecimal(getDoubleDecimal(attr.step));
    paramAttr = attr;
}

void WDouble::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->nameLabel->width();
    m_impl->nameLabel->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WDouble::setValueWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->valueWidget->width();
    m_impl->valueWidget->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WDouble::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
void WDouble::resetValue()
{
    setValue(m_impl->min.first());
    // m_impl->valueWidget->setText(QString::number(m_impl->min));
}
void WDouble::setValue(const QVariant& value)
{
    auto valueString = value.toString();
    if (valueString.isEmpty())
    {
        return;
    }

    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");

    m_impl->valueWidget->setText(valueString);
}

QString WDouble::text() { return m_impl->valueWidget->text().trimmed(); }

QVariant WDouble::value()
{
    // return ui->valueWidget->text().trimmed();
    return m_impl->isValid ? m_impl->value : m_impl->min.first();
}
QVariant WDouble::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
void WDouble::setMinNum(const QVariant& minNum)
{
    if (m_impl->max.isEmpty())
    {
        setNumRange(minNum, QVariant(Limit::max<quint64>()));
        return;
    }
    QString maxStr;
    for (auto& item : m_impl->max)
    {
        maxStr += QString("%1|").arg(item, 0, 'f');
    }
    maxStr.chop(1);
    setNumRange(minNum, QVariant(maxStr));
}

void WDouble::setMaxNum(const QVariant& maxNum)
{
    if (m_impl->min.isEmpty())
    {
        setNumRange(QVariant(Limit::min<quint64>()), maxNum);
        return;
    }
    QString minStr;
    for (auto& item : m_impl->min)
    {
        minStr += QString("%1|").arg(item, 0, 'f');
    }
    minStr.chop(1);
    setNumRange(QVariant(minStr), maxNum);
}

void WDouble::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    m_impl->min.clear();
    m_impl->max.clear();
    auto minList = minNum.toString().split("|");
    auto maxList = maxNum.toString().split("|");
    auto size = minList.size() > maxList.size() ? maxList.size() : minList.size();
    if (size <= 0)
    {
        m_impl->min << 0;
        m_impl->max << static_cast<double>(Limit::max<qint64>());
    }
    else
    {
        bool convertMin = false;
        bool convertMax = false;
        for (int i = 0; i < size; ++i)
        {
            auto& minStr = minList.at(i);
            auto& maxStr = maxList.at(i);
            auto tempMin = minStr.toDouble(&convertMin);
            auto tempMax = maxStr.toDouble(&convertMax);
            if (!convertMax || !convertMin)
            {
                continue;
            }
            m_impl->min << tempMin;
            m_impl->max << tempMax;
        }
        if (m_impl->min.isEmpty())
        {
            //            m_impl->min << 0;
            //            m_impl->max << static_cast<double>(Limit::max<qint64>());
            m_impl->min << getMin(m_impl->paraAttr.attributeType);
            m_impl->max << getMax(m_impl->paraAttr.attributeType);
        }
    }
    /* 生成Tips */
    size = m_impl->min.size();
    QString tip;
    QRegExp rx(R"((\.){0,1}0+$)");
    for (int i = 0; i < size; ++i)
    {
        QString minText = QString("%1").arg(m_impl->min.at(i), 0, 'f').replace(rx, "");
        QString maxText = QString("%1").arg(m_impl->max.at(i), 0, 'f').replace(rx, "");
        tip += QString("%1~%2,").arg(minText).arg(maxText);
    }
    tip.chop(1);
    tip = QString("当前输入范围为%1").arg(tip);
    m_impl->valueWidget->setToolTip(tip);

    // 默认值为最小值
    setValue(m_impl->min.first());
    // 当修改的值与原值相等时也触发textChanged信号,更新超出范围的样式
    if (m_impl->min.first() == m_impl->value)
        emit m_impl->valueWidget->textChanged(m_impl->valueWidget->text());

    m_impl->value = m_impl->min.first();
    m_impl->oldValue = QString::number(m_impl->value);
    m_impl->isValid = true;
}

void WDouble::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WDouble::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WDouble::setParamDesc(const QString& desc) { m_impl->nameLabel->setText(desc); }
void WDouble::setUnit(const QString& unit)
{
    auto tempUnit = unit.trimmed();
    m_impl->unitLabel->setText(tempUnit);
    QFont font = m_impl->unitLabel->font();
    font.setPointSize(16);
    QFontMetrics fm(font);
    auto rect = fm.boundingRect(tempUnit);

    auto unitWidth = rect.width() + UnitExtendedWidth;
    m_impl->unitLabel->setFixedWidth(unitWidth);
    m_impl->valueWidget->setTextMargins(0, 0, unitWidth, 0);
}
void WDouble::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}
void WDouble::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}
void WDouble::textChanged(const QString& newValue)
{
    if (!matchingDouble(newValue, m_impl->regDecimal, m_impl->oldValue))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    /*
     * -1 true
     * -1. true
     * -01. true
     * -01.1545 true
     */
    auto newValueNum = m_impl->oldValue.toDouble();

    /* 验证范围 */
    if (!checkValueRange(m_impl->min, m_impl->max, newValueNum))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    m_impl->isValid = true;
    m_impl->value = newValueNum;
    outOfRangesLineEdit(m_impl->valueWidget, false);
}
void WDouble::setDecimal(int dec)
{
    if (dec < 0)
    {
        return;
    }
    m_impl->regDecimal = QString(R"(^-?[0-9]\d*\.?\d{0,%1}$)").arg(dec);
}
void WDouble::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
