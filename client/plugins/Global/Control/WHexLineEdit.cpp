#include "WHexLineEdit.h"
#include "CConverter.h"
#include "ControlAttribute.h"
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QToolTip>

static bool matchingHex(const QString& str)
{
    QRegExp reg(R"(^([0-9a-fA-F][\da-fA-F]*|0)$)");
    return reg.exactMatch(str);
}

class WHexLineEditImpl
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

WHexLineEdit::WHexLineEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WHexLineEditImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    //   m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->valueWidget->setCursorPosition(0);

    QRegExp reg(R"(^([0-9a-fA-F][\da-fA-F]*|0)$)");
    m_impl->valueWidget->setValidator(new QRegExpValidator(reg, this));

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

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WHexLineEdit::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WHexLineEdit::textChanged);
}

WHexLineEdit::~WHexLineEdit() { delete m_impl; }

void WHexLineEdit::setReadOnly(bool readOnly) { m_impl->valueWidget->setReadOnly(readOnly); }

void WHexLineEdit::setNameWidth(int w)
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
void WHexLineEdit::setValueWidth(int w)
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
void WHexLineEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
void WHexLineEdit::setMaxLength(int len) { m_impl->maxHexLength = len <= 0 ? m_impl->maxHexLength : len; }

void WHexLineEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    m_impl->paraAttr = attr;
    setMaxLength(m_impl->paraAttr.length * 2);

    setObjectName(attr.id);

    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue.toString().isEmpty() ? m_impl->min : attr.initValue);
}

void WHexLineEdit::resetValue() { setValue(m_impl->min); }

void WHexLineEdit::setValue(const QVariant& value)
{
    auto valueString = value.toString();

    //    valueString.remove(QRegExp(R"(^0+)"));                                                                     // 去除前面所有的0
    //    m_impl->valueWidget->setText(QString("%1").arg(valueString, m_impl->maxHexLength, QChar('0')).toUpper());  // 再按照最大长度补齐

    m_impl->valueWidget->setText(valueString.toUpper());
}

QVariant WHexLineEdit::value() { return m_impl->isValid ? m_impl->valueWidget->text() : m_impl->min; }

QVariant WHexLineEdit::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->valueWidget->text();
}

QString WHexLineEdit::text() { return m_impl->valueWidget->text().trimmed(); }

void WHexLineEdit::setMinNum(const QVariant& minNum) { setNumRange(minNum, QVariant(m_impl->max)); }

void WHexLineEdit::setMaxNum(const QVariant& maxNum) { setNumRange(QVariant(m_impl->min), maxNum); }

void WHexLineEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}
void WHexLineEdit::setUnit(const QString& unit)
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

void WHexLineEdit::setNumRange(const QVariant& minNum, const QVariant& maxNum)
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

    m_impl->max = convertMax ? maxNum.toString().toUpper() : QString::number(typeMax, 16).toUpper();
    m_impl->min = convertMin ? minNum.toString().toUpper() : QString::number(typeMin, 16).toUpper();

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

void WHexLineEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WHexLineEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WHexLineEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WHexLineEdit::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
}

void WHexLineEdit::textChanged(const QString& newValue)
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
