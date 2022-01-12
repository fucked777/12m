
#include "WStatusLabel.h"
#include "ControlAttribute.h"
#include <QLabel>

static QString getCurrentUnicode(const QByteArray& ba)
{
    QTextCodec::ConverterState state;
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(ba.constData(), ba.size(), &state);

    if (state.invalidChars > 0)
    {
        text = QTextCodec::codecForName("GBK")->toUnicode(ba);
    }
    else
    {
        text = ba;
    }
    return text;
}

class WStatusLabelImpl
{
public:
    QLabel* valueWidget{ nullptr };
    QLabel* unitLabel{ nullptr };
    QLabel* nameLabel{ nullptr };
    ParameterAttribute paraAttr;
};

WStatusLabel::WStatusLabel(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WStatusLabelImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->unitLabel = new QLabel(this);
    m_impl->valueWidget = new QLabel(this);

    m_impl->valueWidget->setStyleSheet("QLabel {color: black;}");

    m_impl->valueWidget->setEnabled(false);
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

WStatusLabel::~WStatusLabel() { delete m_impl; }

void WStatusLabel::setNameWidth(int w) { m_impl->nameLabel->setFixedWidth(w); }
void WStatusLabel::setValueWidth(int w) { m_impl->valueWidget->setFixedWidth(w); }
void WStatusLabel::setControlHeight(int h)
{
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

bool WStatusLabel::eventFilter(QObject* watched, QEvent* event)
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

void WStatusLabel::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */)
{
    paramAttr = attr;
    m_impl->paraAttr = attr;

    setObjectName(attr.id);
    setName(attr.desc);
    setUnit(attr.unit);
    setValue(attr.initValue);
}

void WStatusLabel::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WStatusLabel::setUnit(const QString& unit) { m_impl->unitLabel->setText(unit.trimmed()); }

void WStatusLabel::setValue(const QVariant& value)
{
    QString valueString;
    if (value.type() == QVariant::Double)  // 小数的先转为Double，再转为QString
    {
        // 去掉小数后多于的0
        QRegExp rx(R"((\.){0,1}0+$)");
        valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");
    }
    else
    {
        valueString = value.toString();
    }

    if (valueString.isEmpty())
    {
        return;
    }
    if (m_impl->paraAttr.displayFormat == DisplayFormat::DisplayFormat_Hex)
    {
        m_impl->valueWidget->setText(valueString.toUpper());
    }
    else
    {
        if (m_impl->paraAttr.id == "PseudoCodeNumber")  //伪码服务器里伪码编号参数需要特殊处理显示
        {
            auto size = value.toString().size();
            valueString.resize(size);
            for (int i = 0; i < size; ++i)
            {
                valueString[i] = value.toString()[size - 1 - i];
            }
        }
        m_impl->valueWidget->setText(valueString);
    }
}

QVariant WStatusLabel::value() { return m_impl->valueWidget->text().trimmed(); }

QVariant WStatusLabel::value(bool& isValid)
{
    isValid = true;
    return value();
}
void WStatusLabel::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
