#include "WHexLineEditOnly.h"

#include "CConverter.h"
#include "ControlAttribute.h"
#include "WHexLineEditOnly.h"
#include <QLabel>
#include <QLineEdit>
#include <QToolTip>

static bool matchingHex(const QString& str)
{
    QRegExp reg(R"(^([0-9a-fA-F][\da-fA-F]*|0)$)");
    return reg.exactMatch(str);
}

class WHexLineEditOnlyImpl
{
public:
    QLineEdit* valueWidget{ nullptr };
    QLabel* unitLabel{ nullptr };
    QString max{};
    QString min{};
    ParameterAttribute paraAttr;
    QString oldValue;
    bool isValid{ false };
    int maxHexLength{};  // 最大值以十六进制表示时的长度，用来填充0
};

WHexLineEditOnly::WHexLineEditOnly(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WHexLineEditOnlyImpl)
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
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->valueWidget);

    setNumRange("-", "-");

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WHexLineEditOnly::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WHexLineEditOnly::textChanged);
}

WHexLineEditOnly::~WHexLineEditOnly() { delete m_impl; }

void WHexLineEditOnly::setReadOnly(bool readOnly) { m_impl->valueWidget->setReadOnly(readOnly); }

void WHexLineEditOnly::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
}
void WHexLineEditOnly::setValueWidth(int w)
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
void WHexLineEditOnly::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
void WHexLineEditOnly::setMaxLength(int len) { m_impl->maxHexLength = len <= 0 ? m_impl->maxHexLength : len; }

void WHexLineEditOnly::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    m_impl->paraAttr = attr;
    paramAttr = attr;
    setMaxLength(m_impl->paraAttr.length * 2);

    setObjectName(attr.id);

    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue.isValid() ? attr.initValue : attr.minValue);
}

void WHexLineEditOnly::resetValue() { setValue(m_impl->min); }

void WHexLineEditOnly::setValue(const QVariant& value)
{
    auto valueString = value.toString();

    valueString.remove(QRegExp(R"(^0+)"));                                                                     // 去除前面所有的0
    m_impl->valueWidget->setText(QString("%1").arg(valueString, m_impl->maxHexLength, QChar('0')).toUpper());  // 再按照最大长度补齐
}

QVariant WHexLineEditOnly::value() { return m_impl->isValid ? m_impl->valueWidget->text() : m_impl->min; }

QVariant WHexLineEditOnly::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->valueWidget->text();
}

QString WHexLineEditOnly::text() { return m_impl->valueWidget->text().trimmed(); }

void WHexLineEditOnly::setMinNum(const QVariant& minNum) { setNumRange(minNum, QVariant(m_impl->max)); }

void WHexLineEditOnly::setMaxNum(const QVariant& maxNum) { setNumRange(QVariant(m_impl->min), maxNum); }

void WHexLineEditOnly::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
}
void WHexLineEditOnly::setUnit(const QString& unit)
{
    auto tempUnit = unit.trimmed();
    m_impl->unitLabel->setText(tempUnit);

    QFont font = m_impl->unitLabel->font();
    font.setPointSize(12);
    QFontMetrics fm(font);
    auto rect = fm.boundingRect(tempUnit);

    auto unitWidth = rect.width() + UnitExtendedWidth;
    m_impl->unitLabel->setFixedWidth(unitWidth);
    m_impl->valueWidget->setTextMargins(0, 0, unitWidth, 0);
}

void WHexLineEditOnly::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    bool convertMin = false;
    bool convertMax = false;

    // 传入的最大最小值按十六进制处理
    auto tempMax = maxNum.toString().toULongLong(&convertMax, 16);
    auto tempMin = minNum.toString().toULongLong(&convertMin, 16);

    // 设置的最大最小值不能大于或者小于该类型的最大最小值
    auto typeMax = getMax(m_impl->paraAttr.attributeType);
    auto typeMin = getMin(m_impl->paraAttr.attributeType);
    if (tempMin < typeMin)
    {
        tempMin = typeMin;
    }
    if (tempMax > typeMax)
    {
        tempMax = typeMax;
    }
    /* 20210904 wp?? */
    m_impl->max = convertMax ? QString::number(tempMax, 16).toUpper() : QString::number(typeMax, 16).toUpper();
    m_impl->min = convertMin ? QString::number(tempMin, 16).toUpper() : QString::number(typeMin, 16).toUpper();

    m_impl->maxHexLength = convertMax ? m_impl->max.length() : m_impl->maxHexLength;

    // 十六进制显示最大最小值
    QString tip = QString("当前输入范围为%1~%2").arg(m_impl->min).arg(m_impl->max);
    m_impl->valueWidget->setToolTip(tip);

    m_impl->isValid = true;
    auto oldValue = m_impl->valueWidget->text();  // 记录原值字符串
    // 调用方法，补齐0，并且判断原值是否在新范围内
    setValue(oldValue);

    // 调用setValue后，如果和原值一样，也触发一次信号，更新超出范围的样式
    if (oldValue == m_impl->valueWidget->text())
        emit textChanged(m_impl->valueWidget->text());
}

void WHexLineEditOnly::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);
}

void WHexLineEditOnly::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }

void WHexLineEditOnly::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WHexLineEditOnly::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
}

void WHexLineEditOnly::textChanged(const QString& newValue)
{
    if (!matchingHex(newValue))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    if (newValue.length() > m_impl->maxHexLength)
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    auto newValNum = newValue.toULongLong(nullptr, 16);
    auto maxValNum = m_impl->max.toULongLong(nullptr, 16);
    auto minValNum = m_impl->min.toULongLong(nullptr, 16);
    // 验证范围
    if (((newValNum > maxValNum) || (newValNum < minValNum)))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    m_impl->oldValue = newValue.toUpper();
    m_impl->isValid = true;
    outOfRangesLineEdit(m_impl->valueWidget, false);
}
