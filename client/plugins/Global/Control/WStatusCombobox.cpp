
#include "WStatusCombobox.h"
#include "ControlAttribute.h"

class WStatusComboboxImpl
{
public:
    QLabel* valueWidget{ nullptr };
    QLabel* unitLabel{ nullptr };
    QLabel* nameLabel{ nullptr };

    QMap<QVariant, QString> enumValueMap;
    QVariant value;
};

WStatusCombobox::WStatusCombobox(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WStatusComboboxImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLabel(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    //  m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(StatusputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);
    layout->addWidget(m_impl->unitLabel);
    layout->addStretch();

    this->setFixedWidth(ControlWidgetS);
    this->setMaximumHeight(ControlHeight);

    m_impl->valueWidget->setMouseTracking(true);
    m_impl->valueWidget->installEventFilter(this);
}

WStatusCombobox::~WStatusCombobox() { delete m_impl; }

void WStatusCombobox::setNameWidth(int w) { m_impl->nameLabel->setFixedWidth(w); }
void WStatusCombobox::setValueWidth(int w) { m_impl->valueWidget->setFixedWidth(w); }
void WStatusCombobox::setControlHeight(int h)
{
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

bool WStatusCombobox::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_impl->valueWidget)
    {
        if (event->type() == QEvent::MouseMove)
        {
            m_impl->valueWidget->setToolTip(m_impl->valueWidget->text());
        }
    }

    return WWidget::eventFilter(watched, event);
}

void WStatusCombobox::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    paramAttr = attr;
    for (auto iter = map.begin(); iter != map.end(); ++iter)
    {
        if (iter.key() != QString("无此参数"))
        {
            (m_impl->enumValueMap)[iter.value()] = iter.key().trimmed();
        }
        else
        {
            (m_impl->enumValueMap)[iter.value()] = "";
        }
    }

    setObjectName(attr.id);

    setName(attr.desc);
    setUnit(attr.unit);
    setValue(attr.initValue);

    //    if (m_impl->nameLabel->text().contains("工作模式"))
    //    {
    //        this->setFixedWidth(380);
    //    }
    //    else
    //    {
    //        this->setFixedWidth(ControlWidgetS);
    //    }
}

void WStatusCombobox::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}
void WStatusCombobox::setUnit(const QString& unit) { m_impl->unitLabel->setText(unit.trimmed()); }

void WStatusCombobox::setValue(const QVariant& value)
{
    auto find = m_impl->enumValueMap.find(value);
    if (find != m_impl->enumValueMap.end())
    {
        m_impl->value = find.key();
        m_impl->valueWidget->setText(find.value());
    }
    else
    {
        if (!m_impl->enumValueMap.isEmpty())
        {
            auto begin = m_impl->enumValueMap.begin();
            m_impl->value = begin.key();
            m_impl->valueWidget->setText(begin.value());
        }
    }
}

QVariant WStatusCombobox::value() { return m_impl->value; }
QVariant WStatusCombobox::value(bool& isValid)
{
    isValid = true;
    return value();
}

void WStatusCombobox::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
