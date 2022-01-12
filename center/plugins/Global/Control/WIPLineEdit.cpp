#include "WIPLineEdit.h"
#include "CConverter.h"
#include "ControlAttribute.h"
#include <QLabel>
#include <QLineEdit>
#include <QToolTip>

#include "CustomIPLineEdit.h"

static bool matchingIP(const QString& str)
{
    QRegExp reg("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?["
                "0-9][0-9]?)\\b");
    return reg.exactMatch(str);
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

static QString Hex2IP(const QString& hexIP)
{
    if (hexIP.toLongLong(nullptr, 16) == 0)
    {
        return QString("0.0.0.0");
    }

    // 四字节，十六进制是8个字符，每两个表示一个ip段，每段不足补0
    if (hexIP.length() != 8)
        return "";

    QStringList ipList;
    int i = 0;
    for (i = 0; i < 8; i += 2)
    {
        ipList << QString::number(hexIP.mid(i, 2).toUInt(0, 16));
    }
    auto ipStr = ipList.join(".");

    return matchingIP(ipStr) ? ipStr : "";
}

class WIPLineEditImpl
{
public:
    CustomIPLineEdit* valueWidget{ nullptr };
    QLabel* nameLabel{ nullptr };
    quint64 value{};
    quint64 max{};
    quint64 min{};
    ParameterAttribute paraAttr;
    QString oldValue;
    bool isValid{ false };
};

WIPLineEdit::WIPLineEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WIPLineEditImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->valueWidget = new CustomIPLineEdit(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth - 10);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth + 10);
    //   m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->valueWidget->setCursorPosition(0);

    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    setNumRange(0x00000000, 0xFFFFFFFF);

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WIPLineEdit::editingFinished);
    connect(m_impl->valueWidget, &CustomIPLineEdit::textChanged, this, &WIPLineEdit::textChanged);
}

WIPLineEdit::~WIPLineEdit() { delete m_impl; }

void WIPLineEdit::setNameWidth(int w)
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

void WIPLineEdit::setValueWidth(int w)
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

void WIPLineEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WIPLineEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    m_impl->paraAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setNumRange(attr.minValue, attr.maxValue);
    setUnit(attr.unit);
    setValue(attr.initValue);
}

void WIPLineEdit::resetValue() { setValue(m_impl->min); }

void WIPLineEdit::setValue(const QVariant& value)
{
    auto valueString = value.toString();

    if (!matchingIP(valueString))
    {
        valueString = QString("0.0.0.0");
    }

    m_impl->valueWidget->setText(valueString);
}

QVariant WIPLineEdit::value() { return m_impl->valueWidget->text(); }

QVariant WIPLineEdit::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return m_impl->valueWidget->text();
}

QString WIPLineEdit::text() { return m_impl->valueWidget->text().trimmed(); }

void WIPLineEdit::setMinNum(const QVariant& minNum) { setNumRange(minNum, QVariant(m_impl->max)); }

void WIPLineEdit::setMaxNum(const QVariant& maxNum) { setNumRange(QVariant(m_impl->min), maxNum); }

void WIPLineEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WIPLineEdit::setNumRange(const QVariant& minNum, const QVariant& maxNum)
{
    auto tempMinNum = minNum;
    auto tempMaxNum = maxNum;
    if (tempMinNum.toString().isEmpty())
    {
        tempMinNum = "0.0.0.0";
    }
    if (tempMaxNum.toString().isEmpty())
    {
        tempMaxNum = "255.255.255.255";
    }

    m_impl->min = IP2Hex(tempMinNum.toString()).toULongLong(nullptr, 16);
    m_impl->max = IP2Hex(tempMaxNum.toString()).toULongLong(nullptr, 16);

    quint64 min = 0x00000000;
    quint64 max = 0xFFFFFFFF;

    if (m_impl->min < min)
    {
        m_impl->min = min;
    }
    if (m_impl->max > max)
    {
        m_impl->max = max;
    }

    auto hexMin = QString::number(m_impl->min, 16);
    auto hexMax = QString::number(m_impl->max, 16);

    QString tip = QString("当前输入范围为%1~%2").arg(Hex2IP(hexMin)).arg(Hex2IP(hexMax));
    m_impl->valueWidget->setToolTip(tip);

    m_impl->isValid = true;
}

void WIPLineEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WIPLineEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WIPLineEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WIPLineEdit::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }

    m_impl->valueWidget->setText(m_impl->oldValue);
    emit sig_valueChange(m_impl->oldValue);
    emit sig_valueChange(QVariant(m_impl->value));
}

void WIPLineEdit::textChanged(const QString& newValue)
{
    if (!m_impl->valueWidget->isValidIP())
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    auto newValueNum = IP2Hex(newValue).toULongLong(nullptr, 16);
    /* 验证范围 */
    if (((newValueNum > m_impl->max) || (newValueNum < m_impl->min)))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    m_impl->value = newValueNum;
    m_impl->isValid = true;
    outOfRangesLineEdit(m_impl->valueWidget, false);
}
