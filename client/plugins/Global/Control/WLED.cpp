#include "WLED.h"

#include "ControlAttribute.h"

#include <QLabel>

static constexpr const char* redQss = "QLabel {"
                                      "background-image: url(:/src/dev_unlock.png);"
                                      "background-position:center;"
                                      "background-repeat:no-repeat;"
                                      "border-style: none;"
                                      "}";

static constexpr const char* greenQss = "QLabel {"
                                        "background-image: url(:/src/dev_locked.png);"
                                        "background-position:center;"
                                        "background-repeat:no-repeat;"
                                        "border-style: none;"
                                        "}";

static constexpr const char* grayQss = "QLabel {"
                                       "background-image: url(:/src/dev_disable.png);"
                                       "background-position:center;"
                                       "background-repeat:no-repeat;"
                                       "border-style: none;"
                                       "}";

class WLEDImpl
{
public:
    QLabel* valueWidget{ nullptr };
    QLabel* nameLabel{ nullptr };
    QVariant value{};
    ParameterAttribute paraAttr{};

    QMap<QVariant, QString> ledEnumMap;  // QMap<值, 对应的颜色>
};

WLED::WLED(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WLEDImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->valueWidget = new QLabel(this);

    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->valueWidget->setFixedSize(ControlStatusLEDWidth, ControlStatusLEDHeight);
    //   m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(StatusputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);
    layout->addStretch();

    this->setFixedWidth(ControlWidgetS);
    this->setMaximumHeight(ControlHeight);
}

WLED::~WLED() { delete m_impl; }

void WLED::setNameWidth(int w) { m_impl->nameLabel->setFixedWidth(w); }
void WLED::setValueWidth(int w) { m_impl->valueWidget->setFixedWidth(w); }
void WLED::setControlHeight(int h)
{
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WLED::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setValue(attr.initValue);
    m_impl->paraAttr = attr;

    for (auto key : map.keys())
    {
        QStringList strList = key.split(":");
        if (strList.size() >= 2)
        {
            m_impl->ledEnumMap.insert(map.value(key), strList.last());
        }
    }
    setValue(-1);
}

void WLED::setValue(const QVariant& value)
{
    if (value == m_impl->value)
    {
        return;
    }
    m_impl->value = value;

    auto color = m_impl->ledEnumMap.value(value);
    QString qssStr;
    if (color == "R")
    {
        qssStr = redQss;
    }
    else if (color == "G")
    {
        qssStr = greenQss;
    }
    else if (color == "Y")
    {
        qssStr = grayQss;
    }
    else
    {
        qssStr = grayQss;
    }
    m_impl->valueWidget->setStyleSheet(qssStr);
}

void WLED::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

QVariant WLED::value() { return m_impl->value; }
QVariant WLED::value(bool& isValid)
{
    isValid = true;
    return value();
}
