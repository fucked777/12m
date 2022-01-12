#include "WHexStringEdit.h"

#include "CConverter.h"
#include "ControlAttribute.h"
#include <QLabel>
#include <QLineEdit>
#include <QToolTip>

static bool matchingHex(const QString& str)
{
    QRegExp reg(R"(^([0-9a-fA-F][\da-fA-F]*|0)$)");
    return reg.exactMatch(str);
}

class WHexStringEditImpl
{
public:
    QLineEdit* valueWidget{ nullptr };
    QLabel* unitLabel{ nullptr };
    QLabel* nameLabel{ nullptr };
    QString max{};
    QString min{};
    ParameterAttribute paraAttr;
    QString oldValue;
    bool isValid{ false };
    int maxHexLength{};  // 最大值以十六进制表示时的长度，用来填充0
};

WHexStringEdit::WHexStringEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WHexStringEditImpl)
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
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    setNumRange("-", "-");

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WHexStringEdit::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WHexStringEdit::textChanged);
}

WHexStringEdit::~WHexStringEdit() { delete m_impl; }

void WHexStringEdit::setReadOnly(bool readOnly) { m_impl->valueWidget->setReadOnly(readOnly); }

void WHexStringEdit::setNameWidth(int w)
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
void WHexStringEdit::setValueWidth(int w)
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
void WHexStringEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
void WHexStringEdit::setMaxLength(int len) { m_impl->maxHexLength = len <= 0 ? m_impl->maxHexLength : len; }

void WHexStringEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    m_impl->paraAttr = attr;
    setMaxLength(m_impl->paraAttr.length * 2);

    setObjectName(attr.id);

    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue.isValid() ? attr.initValue : attr.minValue);
}

void WHexStringEdit::resetValue() { setValue(m_impl->min); }

void WHexStringEdit::setValue(const QVariant& value)
{
    auto valueString = value.toString();
    if (valueString.isEmpty())
    {
        return;
    }

    valueString.remove(QRegExp(R"(^0+)"));                                                                     // 去除前面所有的0
    m_impl->valueWidget->setText(QString("%1").arg(valueString, m_impl->maxHexLength, QChar('0')).toUpper());  // 再按照最大长度补齐
}

QVariant WHexStringEdit::value() { return m_impl->isValid ? m_impl->valueWidget->text() : m_impl->min; }

QVariant WHexStringEdit::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->valueWidget->text();
}
QString WHexStringEdit::text() { return m_impl->valueWidget->text().trimmed(); }

void WHexStringEdit::setMinNum(const QVariant& minNum) { setNumRange(minNum, QVariant(m_impl->max)); }

void WHexStringEdit::setMaxNum(const QVariant& maxNum) { setNumRange(QVariant(m_impl->min), maxNum); }

void WHexStringEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}
void WHexStringEdit::setUnit(const QString& unit)
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

void WHexStringEdit::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    // 传入的最大最小值按十六进制处理
    auto tempMax = maxNum.toString();
    auto tempMin = minNum.toString();

    if (!matchingHex(tempMax) || tempMax.size() > m_impl->paraAttr.length * 2)
    {
        tempMax = QString("%1").arg(0xF, m_impl->paraAttr.length * 2, 16, QChar('F')).toUpper();
    }
    if (!matchingHex(tempMin) || tempMin.size() > m_impl->paraAttr.length * 2)
    {
        tempMin = "0";
    }

    m_impl->max = tempMax.toUpper();
    m_impl->min = tempMin.toUpper();

    m_impl->maxHexLength = m_impl->max.length();

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

void WHexStringEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WHexStringEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WHexStringEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WHexStringEdit::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
}

void WHexStringEdit::textChanged(const QString& newValue)
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

    // 验证范围
    if (newValue.length() < m_impl->min.length() || newValue.compare(m_impl->min, Qt::CaseInsensitive) < 0)
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    if (newValue.length() > m_impl->max.length() || newValue.compare(m_impl->max, Qt::CaseInsensitive) > 0)
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    m_impl->oldValue = newValue.toUpper();
    m_impl->isValid = true;
    outOfRangesLineEdit(m_impl->valueWidget, false);
}
