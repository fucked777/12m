#include "WTimeEdit.h"

#include "ControlAttribute.h"

#include <QTimeEdit>

class WTimeEditImpl
{
public:
    ParameterAttribute paraAttr;
    QTimeEdit* valueWidget{ nullptr };
    QString value{};
    QLabel* nameLabel{ nullptr };
};

WTimeEdit::WTimeEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WTimeEditImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->valueWidget = new QTimeEdit(this);
    m_impl->valueWidget->setDisplayFormat("hh时mm分ss秒zzz毫秒");

    m_impl->valueWidget->setCalendarPopup(true);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    // m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
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

    connect(m_impl->valueWidget, &QTimeEdit::timeChanged, this, &WTimeEdit::timeChanged);
}

WTimeEdit::~WTimeEdit() { delete m_impl; }

void WTimeEdit::setNameWidth(int w)
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
void WTimeEdit::setValueWidth(int w)
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
void WTimeEdit::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WTimeEdit::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);

    if (attr.initValue.toString().isEmpty())
    {
        // 设置当前时间
        if (attr.desc.indexOf("结束") >= 0)  // 结束时间比当前时间快20分钟  便于下发任务
            setValue(GlobalData::currentTime().addSecs(20 * 60).toString(TIME_DISPLAY_FORMAT));
        else  //开始时间比当前时间快5分钟
            setValue(GlobalData::currentTime().addSecs(5 * 60).toString(TIME_DISPLAY_FORMAT));
    }
    else
    {
        setValue(attr.initValue);
    }
}

void WTimeEdit::resetValue() { m_impl->valueWidget->setTime(GlobalData::currentTime()); }

void WTimeEdit::setValue(const QVariant& value)
{
    auto time = QTime::fromString(value.toString(), TIME_DISPLAY_FORMAT);
    m_impl->valueWidget->setTime(time);
}
void WTimeEdit::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

QVariant WTimeEdit::value() { return m_impl->value; }

QVariant WTimeEdit::value(bool& isValid)
{
    isValid = m_impl->valueWidget->time().isValid();

    return m_impl->value;
}

QString WTimeEdit::text() { return m_impl->valueWidget->time().toString(); }

void WTimeEdit::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WTimeEdit::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
void WTimeEdit::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WTimeEdit::timeChanged(const QTime& time)
{
    if (!time.isValid())
    {
        outOfRangesLineEdit(m_impl->valueWidget, true);
        return;
    }
    m_impl->value = time.toString(TIME_DISPLAY_FORMAT);

    QVariant data = value();
    emit sig_valueChange(data);
}
