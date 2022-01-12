#include "WConverterWdBm.h"
#include "CConverter.h"
#include "ControlAttribute.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolTip>

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

class WConverterWdBmImpl
{
public:
    ParameterAttribute setAttr;
    /* 当前是否是dBm */
    bool isDBm{ true };
    /* 当前value */
    double value{};
    double max{};
    double min{};
    /* 默认小数点后3位精度 */
    QString regDecimal;
    QString oldValue;
    QLineEdit* valueWidget{ nullptr };
    QLabel* unitLabel{ nullptr };
    QLabel* nameLabel{ nullptr };
    QPushButton* switchBtn{ nullptr };
    bool isValid{ false };
};

WConverterWdBm::WConverterWdBm(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WConverterWdBmImpl }
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

    m_impl->switchBtn = new QPushButton("切换");
    m_impl->switchBtn->setStyleSheet("QPushButton{min-width: 50px;min-height: 20px;}");
    m_impl->switchBtn->setMaximumWidth(35);
    connect(m_impl->switchBtn, &QPushButton::clicked, this, &WConverterWdBm::slotSwitchBtnClicked);

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 0, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addStretch();
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    lineEditLayout->addWidget(m_impl->switchBtn, 0, Qt::AlignVCenter | Qt::AlignRight);
    lineEditLayout->setDirection(QBoxLayout::Direction::LeftToRight);
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

    /* 默认精度为3位小数 */

    setDecimal(6);

    setNumRange("-", "-");

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WConverterWdBm::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WConverterWdBm::textChanged);
}

WConverterWdBm::~WConverterWdBm() { delete m_impl; }

void WConverterWdBm::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->setAttr = attr;
    paramAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);

    if (m_impl->switchBtn)
    {
        //初始化的时候让界面是W---用户要求
        emit m_impl->switchBtn->clicked();
    }
}

void WConverterWdBm::setNameWidth(int w)
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
void WConverterWdBm::setValueWidth(int w)
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
void WConverterWdBm::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WConverterWdBm::resetValue()
{
    setValue(m_impl->min);
    // m_impl->valueWidget->setText(QString::number(m_impl->min));
}
void WConverterWdBm::setValue(const QVariant& value)
{
    auto valueString = value.toString();
    if (valueString.isEmpty())
    {
        return;
    }

    // 协议上报的是dBm
    if (m_impl->isDBm)
    {
        // 去掉小数后多于的0
        QRegExp rx(R"((\.){0,1}0+$)");
        valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");
        m_impl->valueWidget->setText(valueString);
    }
    else
    {
        m_impl->valueWidget->setText(QString::number(WdBmConversion(value.toDouble(), false)));
    }
}

QString WConverterWdBm::text() { return m_impl->valueWidget->text().trimmed(); }

QVariant WConverterWdBm::value()
{
    QVariant value;
    if (m_impl->isValid)
    {
        auto widgetValue = m_impl->valueWidget->text().toDouble();

        if (m_impl->isDBm)
            value = widgetValue;
        else
            value = WdBmConversion(widgetValue, true);
    }
    else
    {
        value = m_impl->setAttr.minValue;
    }

    return value;
}
QVariant WConverterWdBm::value(bool& isValid)
{
    isValid = m_impl->isValid;

    QVariant value;
    if (m_impl->isValid)
    {
        auto widgetValue = m_impl->valueWidget->text().toDouble();

        if (m_impl->isDBm)
            value = widgetValue;
        else
            value = WdBmConversion(widgetValue, true);
    }
    else
    {
        value = m_impl->setAttr.minValue;
    }

    return value;
}
void WConverterWdBm::setMinNum(const QVariant& minNum) { setNumRange(minNum, QVariant(m_impl->max)); }

void WConverterWdBm::setMaxNum(const QVariant& maxNum) { setNumRange(QVariant(m_impl->min), maxNum); }

void WConverterWdBm::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    bool convertMin = false;
    bool convertMax = false;
    auto tempMax = maxNum.toDouble(&convertMax);
    auto tempMin = minNum.toDouble(&convertMin);

    m_impl->min = convertMin ? tempMin : getMin(m_impl->setAttr.attributeType);
    m_impl->max = convertMax ? tempMax : getMax(m_impl->setAttr.attributeType);

    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    QString minText = QString("%1").arg(m_impl->min, 0, 'f').replace(rx, "");
    QString maxText = QString("%1").arg(m_impl->max, 0, 'f').replace(rx, "");
    QString tip = QString("当前输入范围为%1~%2").arg(minText).arg(maxText);

    m_impl->valueWidget->setToolTip(tip);
    /* 默认值 */
    //    m_impl->valueWidget->setText(QString::number(m_impl->min));
    // 当修改的值与原值相等时也触发textChanged信号,更新超出范围的样式
    if (m_impl->min == m_impl->value)
        emit m_impl->valueWidget->textChanged(m_impl->valueWidget->text());

    //    m_impl->value = m_impl->min;
    //    m_impl->oldValue = QString::number(m_impl->value);
    //    m_impl->isValid = true;
}

void WConverterWdBm::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WConverterWdBm::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}
void WConverterWdBm::setUnit(const QString& unit)
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
void WConverterWdBm::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }
// 修改最的最小值范围、单位
void WConverterWdBm::slotSwitchBtnClicked()
{
    m_impl->isDBm = !m_impl->isDBm;

    double beforeText = m_impl->valueWidget->text().toDouble();
    double afterVal = 0;
    QString unit;
    if (m_impl->isDBm)
    {
        unit = "dBm";
        setNumRange(m_impl->setAttr.minValue, m_impl->setAttr.maxValue);
        afterVal = WdBmConversion(beforeText, true);
    }
    else
    {
        unit = "W";
        setNumRange(WdBmConversion(m_impl->setAttr.minValue.toDouble(), false), WdBmConversion(m_impl->setAttr.maxValue.toDouble(), false));
        afterVal = WdBmConversion(beforeText, false);
    }

    auto afterText = QString::number(afterVal);
    if (afterText.contains("inf"))
        afterText = "0";

    m_impl->valueWidget->setText(afterText);
    setUnit(unit);
}
void WConverterWdBm::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}
void WConverterWdBm::textChanged(const QString& newValue)
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
    if (((newValueNum > m_impl->max) || (newValueNum < m_impl->min)))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    m_impl->isValid = true;
    //    if (m_impl->isDBm)
    //        m_impl->value = newValueNum;
    //    else
    //        m_impl->value = WdBmConversion(newValueNum, true);

    outOfRangesLineEdit(m_impl->valueWidget, false);
}
void WConverterWdBm::setDecimal(int dec)
{
    if (dec < 0)
    {
        return;
    }
    m_impl->regDecimal = QString(R"(^-?[0-9]\d*\.?\d{0,%1}$)").arg(dec);
}
void WConverterWdBm::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
