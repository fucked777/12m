#include "WTracksNumberLineEdit.h"

#include "ControlAttribute.h"
#include <QLabel>
#include <QLineEdit>

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

template<typename T>
class WWTracksNumberLineEditImpl
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

WTracksNumberLineEdit::WTracksNumberLineEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl{ new WWTracksNumberLineEditImpl<double> }
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

    /* 默认精度为3位小数 */

    setDecimal(3);
    setNumRange("", "");

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WTracksNumberLineEdit::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WTracksNumberLineEdit::textChanged);
}

WTracksNumberLineEdit::~WTracksNumberLineEdit() { delete m_impl; }

void WTracksNumberLineEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
    // setDecimal(getDoubleDecimal(attr.step));
    paramAttr = attr;
}

void WTracksNumberLineEdit::setNameWidth(int w)
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
void WTracksNumberLineEdit::setValueWidth(int w)
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
void WTracksNumberLineEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}
void WTracksNumberLineEdit::resetValue()
{
    setValue(m_impl->min.first());
    // m_impl->valueWidget->setText(QString::number(m_impl->min));
}
void WTracksNumberLineEdit::setValue(const QVariant& value)
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

QString WTracksNumberLineEdit::text() { return m_impl->valueWidget->text().trimmed(); }

QVariant WTracksNumberLineEdit::value()
{
    // return ui->valueWidget->text().trimmed();
    return m_impl->isValid ? m_impl->value : m_impl->min.first();
}
QVariant WTracksNumberLineEdit::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->value;
}
void WTracksNumberLineEdit::setMinNum(const QVariant& minNum)
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

void WTracksNumberLineEdit::setMaxNum(const QVariant& maxNum)
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

void WTracksNumberLineEdit::setNumRange(const QVariant& minNum, const QVariant& maxNum)
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

void WTracksNumberLineEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WTracksNumberLineEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WTracksNumberLineEdit::setParamDesc(const QString& desc) { m_impl->nameLabel->setText(desc); }
void WTracksNumberLineEdit::setUnit(const QString& unit)
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
void WTracksNumberLineEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}
void WTracksNumberLineEdit::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}
void WTracksNumberLineEdit::textChanged(const QString& newValue)
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
void WTracksNumberLineEdit::setDecimal(int dec)
{
    if (dec < 0)
    {
        return;
    }
    m_impl->regDecimal = QString(R"(^-?[0-9]\d*\.?\d{0,%1}$)").arg(dec);
}
void WTracksNumberLineEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
