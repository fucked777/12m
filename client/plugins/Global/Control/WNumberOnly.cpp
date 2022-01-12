#include "WNumberOnly.h"

#include "ControlAttribute.h"

#include <QLineEdit>
#include <QRegExp>
#include <QToolTip>

template<typename T>
class WNumberOnlyImpl
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

WUNumberOnly::WUNumberOnly(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WNumberOnlyImpl<quint64> }
{
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    m_impl->valueWidget->setCursorPosition(0);

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 0, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->addWidget(m_impl->valueWidget);

    this->setMaximumHeight(ControlHeight);

    setNumRange("", "");
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WUNumberOnly::textChanged);
    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WUNumberOnly::editingFinished);
}

WUNumberOnly::~WUNumberOnly() { delete m_impl; }

void WUNumberOnly::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
    paramAttr = attr;
}

void WUNumberOnly::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
}
void WUNumberOnly::setValueWidth(int w)
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
void WUNumberOnly::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WUNumberOnly::resetValue() { setValue(m_impl->min.first()); }
void WUNumberOnly::setValue(const QVariant& value)
{
    auto valueString = value.toString();
    if (valueString.isEmpty())
    {
        return;
    }

    m_impl->valueWidget->setText(valueString);
}

QVariant WUNumberOnly::value() { return m_impl->isValid ? m_impl->value : m_impl->min.first(); }

QVariant WUNumberOnly::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
QString WUNumberOnly::text() { return m_impl->valueWidget->text().trimmed(); }

void WUNumberOnly::setMinNum(const QVariant& minNum)
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

void WUNumberOnly::setMaxNum(const QVariant& maxNum)
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

void WUNumberOnly::setNumRange(const QVariant& minNum, const QVariant& maxNum)
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

void WUNumberOnly::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);
}
void WUNumberOnly::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
}

void WUNumberOnly::setParamDesc(const QString& desc) {}
void WUNumberOnly::setUnit(const QString& unit)
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
void WUNumberOnly::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
void WUNumberOnly::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }
void WUNumberOnly::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}

void WUNumberOnly::textChanged(const QString& newValue)
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
WINumberOnly::WINumberOnly(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WNumberOnlyImpl<qint64> }
{
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    m_impl->valueWidget->setCursorPosition(0);

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 1, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->valueWidget);

    // this->setMinimumWidth(ControlWidget);
    this->setFixedWidth(ControlWidgetI);
    // this->setMaximumHeight(ControlHeight);

    setNumRange("", "");
    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WINumberOnly::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WINumberOnly::textChanged);
}

void WINumberOnly::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
}
void WINumberOnly::setValueWidth(int w)
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
void WINumberOnly::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
WINumberOnly::~WINumberOnly() { delete m_impl; }

void WINumberOnly::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
    paramAttr = attr;
}

void WINumberOnly::resetValue() { setValue(m_impl->min.first()); }
void WINumberOnly::setValue(const QVariant& value)
{
    auto valueString = value.toString();

    if (valueString.isEmpty())
    {
        return;
    }
    m_impl->valueWidget->setText(valueString);
}
QString WINumberOnly::text() { return m_impl->valueWidget->text().trimmed(); }

QVariant WINumberOnly::value()
{
    // return ui->valueWidget->text().trimmed();
    return m_impl->isValid ? m_impl->value : m_impl->min.first();
}
QVariant WINumberOnly::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
void WINumberOnly::setMinNum(const QVariant& minNum)
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

void WINumberOnly::setMaxNum(const QVariant& maxNum)
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

void WINumberOnly::setNumRange(const QVariant& minNum, const QVariant& maxNum)
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

void WINumberOnly::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);
}
void WINumberOnly::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
}

void WINumberOnly::setParamDesc(const QString& desc) {}
void WINumberOnly::setUnit(const QString& unit)
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
void WINumberOnly::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
void WINumberOnly::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }
void WINumberOnly::textChanged(const QString& newValue)
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
void WINumberOnly::editingFinished()
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

WDoubleOnly::WDoubleOnly(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WNumberOnlyImpl<double> }
{
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    m_impl->valueWidget->setCursorPosition(0);

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 5, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->valueWidget);

    /* 默认精度为6位小数 */
    setDecimal(6);
    setNumRange("", "");

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WDoubleOnly::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WDoubleOnly::textChanged);
}

WDoubleOnly::~WDoubleOnly() { delete m_impl; }

void WDoubleOnly::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
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

void WDoubleOnly::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
}
void WDoubleOnly::setValueWidth(int w)
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
void WDoubleOnly::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
void WDoubleOnly::resetValue()
{
    setValue(m_impl->min.first());
    // m_impl->valueWidget->setText(QString::number(m_impl->min));
}
void WDoubleOnly::setValue(const QVariant& value)
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

QString WDoubleOnly::text() { return m_impl->valueWidget->text().trimmed(); }

QVariant WDoubleOnly::value()
{
    // return ui->valueWidget->text().trimmed();
    return m_impl->isValid ? m_impl->value : m_impl->min.first();
}
QVariant WDoubleOnly::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
void WDoubleOnly::setMinNum(const QVariant& minNum)
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

void WDoubleOnly::setMaxNum(const QVariant& maxNum)
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

void WDoubleOnly::setNumRange(const QVariant& minNum, const QVariant& maxNum)
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
            m_impl->min << 0;
            m_impl->max << static_cast<double>(Limit::max<qint64>());
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

void WDoubleOnly::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);
}
void WDoubleOnly::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
}

void WDoubleOnly::setParamDesc(const QString& desc) {}
void WDoubleOnly::setUnit(const QString& unit)
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
void WDoubleOnly::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }
void WDoubleOnly::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}
void WDoubleOnly::textChanged(const QString& newValue)
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
void WDoubleOnly::setDecimal(int dec)
{
    if (dec < 0)
    {
        return;
    }
    m_impl->regDecimal = QString(R"(^-?[0-9]\d*\.?\d{0,%1}$)").arg(dec);
}
void WDoubleOnly::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
