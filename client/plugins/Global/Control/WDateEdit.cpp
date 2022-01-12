#include "WDateEdit.h"

#include "WDateEdit.h"

#include "ControlAttribute.h"

#include <QDateEdit>

class WDateEditImpl
{
public:
    ParameterAttribute paraAttr;
    QDateEdit* valueWidget{ nullptr };
    QString value{};
    QLabel* nameLabel{ nullptr };
};

WDateEdit::WDateEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WDateEditImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->valueWidget = new QDateEdit(this);
    m_impl->valueWidget->setDisplayFormat("yyyy年MM月dd日");

    m_impl->valueWidget->setCalendarPopup(true);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    //  m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);

    m_impl->nameLabel->setProperty("nameLabel", "true");
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    connect(m_impl->valueWidget, &QDateEdit::dateChanged, this, &WDateEdit::dateChanged);
}

WDateEdit::~WDateEdit() { delete m_impl; }

void WDateEdit::setNameWidth(int w)
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
void WDateEdit::setValueWidth(int w)
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
void WDateEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WDateEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);

    if (attr.initValue.toString().isEmpty())
    {
        // 设置当前日期
        setValue(GlobalData::currentDate().toString(DATE_DISPLAY_FORMAT));
    }
    else
    {
        setValue(attr.initValue);
    }
}

void WDateEdit::resetValue() { m_impl->valueWidget->setTime(GlobalData::currentTime()); }

void WDateEdit::setValue(const QVariant& value)
{
    auto date = QDate::fromString(value.toString(), DATE_DISPLAY_FORMAT);
    m_impl->valueWidget->setDate(date);
}
void WDateEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

QVariant WDateEdit::value() { return m_impl->value; }

QVariant WDateEdit::value(bool& isValid)
{
    isValid = m_impl->valueWidget->date().isValid();
    return m_impl->value;
}

QString WDateEdit::text() { return m_impl->valueWidget->time().toString(); }

void WDateEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WDateEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WDateEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WDateEdit::dateChanged(const QDate& date)
{
    if (!date.isValid())
    {
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    m_impl->value = date.toString(DATE_DISPLAY_FORMAT);

    QVariant data = value();
    emit sig_valueChange(data);
}
