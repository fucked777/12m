#include "WStringEdit.h"

#include "ControlAttribute.h"

#include <QLineEdit>

class WStringEditImpl
{
public:
    QLineEdit* valueWidget{ nullptr };
    QString rawText;  // 原始数据，没有经过截取的
    QLabel* unitLabel{ nullptr };
    QLabel* nameLabel{ nullptr };
    ParameterAttribute paraAttr;
    bool isValid{ false };
    QString newValue{ "" };
    QVariant value;
    int maxHexLength{};  // 最大值以十六进制表示时的长度，用来填充0
};

static bool matchingHex(const QString& str)
{
    QRegExp reg(R"(^([0-9a-fA-F][\da-fA-F]*|0)$)");
    return reg.exactMatch(str);
}

void WStringEdit::setNameWidth(int w)
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
void WStringEdit::setValueWidth(int w)
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
void WStringEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

WStringEdit::WStringEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WStringEditImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLineEdit(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->unitLabel->setAlignment(Qt::AlignCenter);
    // m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->valueWidget->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    auto lineEditLayout = new QHBoxLayout;
    lineEditLayout->setContentsMargins(0, 0, 0, 0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_impl->unitLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
    m_impl->valueWidget->setLayout(lineEditLayout);

    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this, &WStringEdit::editingFinished);
    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WStringEdit::textChanged);
}

WStringEdit::~WStringEdit() { delete m_impl; }

void WStringEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>&)
{
    paramAttr = attr;
    m_impl->paraAttr = attr;

    setObjectName(attr.id);
    setName(attr.desc);
    setUnit(attr.unit);
    setMaxLength(attr.length);

    setValue(attr.initValue.isValid() ? attr.initValue : attr.minValue);

    m_impl->isValid = true;
}

void WStringEdit::setMaxLength(int length)
{
    if (length <= 0)
    {
        return;
    }

    m_impl->maxHexLength = length;
    m_impl->valueWidget->setMaxLength(length);
}

void WStringEdit::setValue(const QVariant& value)
{
    QString valueString = value.toString();

    if (m_impl->paraAttr.id == "ShortCode" || m_impl->paraAttr.id == "LongCode")  //短码和长码码号界面需要特殊处理显示
    {
        auto size = value.toString().size();
        valueString.resize(size);
        for (int i = 0; i < size; ++i)
        {
            valueString[i] = value.toString()[size - 1 - i];
        }
    }

    if (m_impl->nameLabel->text().indexOf("任务代号") >= 0)  //规定是任务代号必须输入最大长度的数据,故只有任务代号补0
    {
        valueString.remove(QRegExp(R"(^0+)"));                                                                     // 去除前面所有的0
        m_impl->valueWidget->setText(QString("%1").arg(valueString, m_impl->maxHexLength, QChar('0')).toUpper());  // 再按照最大长度补齐
    }
    else
    {
        m_impl->valueWidget->setText(valueString);
    }
}

void WStringEdit::resetValue() { m_impl->valueWidget->clear(); }

QVariant WStringEdit::value(bool& isValid)
{
    isValid = m_impl->isValid;
    return value();
}

QVariant WStringEdit::value()
{
    auto value = m_impl->valueWidget->text().trimmed();

    if (m_impl->paraAttr.id == "ShortCode" || m_impl->paraAttr.id == "LongCode")  //短码和长码码号界面需要特殊处理
    {
        QString valueString;
        int size = value.size();
        valueString.resize(size);
        for (int i = 0; i < size; ++i)
        {
            valueString[i] = value[size - 1 - i];
        }
        value = valueString;
    }

    return value;
}

QString WStringEdit::text() { return m_impl->valueWidget->text().trimmed(); }

void WStringEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}
void WStringEdit::setUnit(const QString& unit)
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

void WStringEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WStringEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WStringEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WStringEdit::textChanged(const QString& newValue)
{
    auto value = newValue.trimmed();
    if (value.isEmpty())
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    if (m_impl->paraAttr.displayFormat == DisplayFormat_HexString && !matchingHex(value))
    {
        m_impl->isValid = false;
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }

    // 特殊处理任务代号，只能是数字 字母 _
    if (m_impl->nameLabel->text().indexOf("任务代号") >= 0)
    {
        m_impl->newValue = QString("%1").arg(value, m_impl->maxHexLength, QChar('0')).toUpper();  //如果是任务代号，则字符必须填满，故填充0
        QRegExp reg(R"([0-9a-zA-Z_-]+)");
        if (!reg.exactMatch(newValue))
        {
            m_impl->isValid = false;
            outOfRangesLineEdit(m_impl->valueWidget, true);
            return;
        }
    }

    //特殊处理对象标识只能是六位 数字 字母
    if (m_impl->nameLabel->text().indexOf("对象标识") >= 0 && m_impl->paraAttr.id == "ObjectIden")
    {
        //        m_impl->newValue =
        //            QString("%1").arg(value, m_impl->paraAttr.maxValue.length(), QChar('0')).toUpper();  //如果是对象标识，则字符必须填满，故填充0
        QRegExp reg(R"([0-9a-zA-Z]{0,6})");
        if (!reg.exactMatch(newValue))
        {
            m_impl->isValid = false;
            outOfRangesLineEdit(m_impl->valueWidget, true);
            return;
        }
    }

    outOfRangesLineEdit(m_impl->valueWidget, false);
    m_impl->isValid = true;
    m_impl->value = value;
}

void WStringEdit::editingFinished()
{
    if (!m_impl->isValid)
    {
        return;
    }
    if (!m_impl->newValue.isEmpty())
    {
        m_impl->valueWidget->setText(m_impl->newValue);
        emit sig_valueChange(m_impl->newValue);
    }
    emit sig_valueChange(QVariant(m_impl->value));
}
