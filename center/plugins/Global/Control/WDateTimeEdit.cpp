#include "WDateTimeEdit.h"
#include "CConverter.h"
#include "ControlAttribute.h"

#include <QDateTimeEdit>
#include <QLabel>
#include <QToolTip>

class WDateTimeEditImpl
{
public:
    QDateTimeEdit* valueWidget{ nullptr };
    QLabel* nameLabel{ nullptr };
    QString value = "";
    bool isValid{ false };
    ParameterAttribute setAttr;
};

WDateTimeEdit::WDateTimeEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WDateTimeEditImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    //   m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);

    m_impl->valueWidget = new QDateTimeEdit(this);
    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->valueWidget->setFixedHeight(ControlHeight);
    m_impl->valueWidget->setDisplayFormat(DATETIME_DISPLAY_FORMAT3);

    m_impl->valueWidget->setCalendarPopup(true);
    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    connect(m_impl->valueWidget, &QDateTimeEdit::dateTimeChanged, this, &WDateTimeEdit::dateTimeChanged);
}

WDateTimeEdit::~WDateTimeEdit() { delete m_impl; }

void WDateTimeEdit::setNameWidth(int w)
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
void WDateTimeEdit::setValueWidth(int w)
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
void WDateTimeEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WDateTimeEdit::setDisplayFormat(const QString& format) { m_impl->valueWidget->setDisplayFormat(format); }

void WDateTimeEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    m_impl->setAttr = attr;

    setObjectName(attr.id);
    setName(attr.desc);

    if (attr.initValue.toString().isEmpty())
    {
        // 设置当前日期时间
        if (attr.desc.indexOf("结束") >= 0)  // 结束时间比当前时间快20分钟  便于下发任务
            setValue(GlobalData::currentDateTime().addSecs(20 * 60).toString(DATETIME_DISPLAY_FORMAT3));
        else  //开始时间比当前时间快5分钟
            setValue(GlobalData::currentDateTime().addSecs(5 * 60).toString(DATETIME_DISPLAY_FORMAT3));
    }
    else
    {
        setValue(attr.initValue);
    }
}

void WDateTimeEdit::setValue(const QVariant& value)
{
    auto date = QDateTime::fromString(value.toString(), DATETIME_DISPLAY_FORMAT3);
    m_impl->valueWidget->setDateTime(date);
}

QVariant WDateTimeEdit::value() { return m_impl->value; }

QVariant WDateTimeEdit::value(bool& isValid)
{
    isValid = m_impl->valueWidget->dateTime().isValid();
    return m_impl->value;
}
QString WDateTimeEdit::text() { return m_impl->valueWidget->text().trimmed(); }

void WDateTimeEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WDateTimeEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WDateTimeEdit::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }

void WDateTimeEdit::dateTimeChanged(const QDateTime& datetime)
{
    if (!datetime.isValid())
    {
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    m_impl->value = datetime.toString(DATETIME_DISPLAY_FORMAT3);

    QVariant data = value();
    emit sig_valueChange(data);
}
void WDateTimeEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
